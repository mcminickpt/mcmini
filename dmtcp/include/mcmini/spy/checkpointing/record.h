#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC_NO_ATOMICS__
#error "C11 atomics are required to compile libmcmini.so"
#else
#include <stdatomic.h>
#endif

#ifndef ATOMIC_BOOL_LOCK_FREE
#error \
    "Atomic booleans must be lock free, but the compiler has indicated this is not the case"
#endif

#ifndef ATOMIC_INT_LOCK_FREE
#error \
    "Atomic integers must be lock free, but the compiler has indicated this is not the case"
#endif

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

/**
 * @brief Describes the different behaviors that `libmcmini.so` should exhibit
 *
 * `libmcmini.so` is pre-loaded (i.e. with LD_PRELOAD) either directly by the
 * `mcmini` process or by `dmtcp_launch` into an executable known as the
 * _target_. The target is the program whose behavior we wish to verify.
 *
 * `libmcmini.so` however will appear within different processes running the
 * target program and must behave differently based on how and when it was
 * loaded. Below is a description on the different ways `libmcmini.so` can
 * behave.
 *
 * I. Classic Model Checking (No Deep Debugging)
 *
 * TARGET_TEMPLATE:
 *   In this mode, `libmcmini.so` enters an infinite loop waiting for the signal
 *   from the `mcmini` process to produce a new branch via `fork()`.
 *
 * TARGET_BRANCH:
 *   In this mode, `libmcmini.so` will now behave as if under the control of the
 * model checker in the `mcmini` process.
 *
 * II. Recording
 *
 * PRE_DMTCP_INIT:
 *    In this mode, the `mcmini` process has `exec()`-ed into `dmtcp_launch`
 * with `libmcmini.so` as a DMTCP plugin. Here, DMTCP will preload mcmini. Prior
 * to DMTCP sending the `DMTCP_EVENT_INIT` to `libmcmini.so`, wrapper functions
 * simply forward calls to `libpthread.so`
 *
 * PRE_CHECKPOINT_THREAD:
 *   In this mode, DMTCP has sent the `DMTCP_EVENT_INIT` event but has not
 * yet created the checkpoint thread. All wrappers (except `pthread_create`)
 * behave as in `PRE_DMTCP_INIT`. For `pthread_create`, the call is _only_
 * forwarded into DMTCP instead -- the checkpoint thread is NOT recorded.
 *
 * CHECKPOINT_THREAD:
 *   In this mode, DMTCP has created the checkpoint thread. The checkpoint
 * thread should not interact with McMini in any way. But the two interact
 * because the functions `libmcmini.so` overrides are used by `dmtcp`
 * extensively (e.g. `sem_wait()`). This mode, special to the library when
 * executing from the perspective of the checkpoint thread, indicates that DMTCP
 * has called directly into McMini. In most cases, this probably means
 * forwarding the call to DMTCP's wrapper functions of to `libpthread.so`.
 *
 * RECORD:
 *   In this mode, `libmcmini.so` performs a light-weight recording of the
 * primitives it encounters and keeps track of their state. Only after doing so
 * do wrapper functions forward calls to the next available function found by
 * `dlsym(3)` using `RTLD_NEXT`.
 *
 * PRE_CHECKPOINT:
 *   In this mode, `libmcmini.so` has been alerted about the end of the
 * recording session. Wrapper functions again forward their calls to the next
 * available function. Wrappers may eventually change their behavior from RECORD
 * mode if necessary.
 *
 * II.i. Deep Debugging with `dmtcp_restart`
 *
 * DMTCP_RESTART_INTO_BRANCH:
 *   In this mode, the DMTCP plugin has been notified of the
 *   `DMTCP_EVENT_RESTART` event. The user-space threads will transition into a
 *   stable state before the template thread and the scheduler process proceed
 *   with model checking. The userspace threads assume that they are immediately
 * under the control of the model checker.
 *
 * II.i. Deep Debugging with `dmtcp_restart`
 *
 * DMTCP_RESTART_INTO_TEMPLATE:
 *   In this mode, the DMTCP plugin has been notified of the
 *   `DMTCP_EVENT_RESTART` event. The user-space threads will transition into a
 *   stable state before the template thread and the scheduler process proceed
 *   with model checking. However, unlike `DMTCP_RESTART_INTO_BRANCH`, the
 * userspace threads assume that they are in a template _process_. The template
 * thread in this template process will repeatedly call `multithreaded_fork()`.
 *
 * TARGET_TEMPLATE_AFTER_RESTART:
 *   In this mode, `libmcmini.so` has been restored by `dmtcp_launch` from a
 *   checkpoint image. Here, `libmcmini.so` will first attempt to communicate
 * with `mcmini` and transfer over the state information recorded during the
 * RECORD phase. After transfer has completed, `libmcmini.so` will behave the
 * same as `TARGET_TEMPLATE`, except this time the template thread will call
 * `multithreaded_fork()` instead of `fork()`. Other userspace threads in the
 * template process are blocked indefinitely and will not communicate with the
 * model checker `mcmini`. Only after a `multithreaded_fork()` will the
 * (duplicates of) these threads participate in model checking.
 *
 * III. Deep Debugging Under the Modeler
 *
 * TARGET_BRANCH_AFTER_RESTART:
 *   In this mode, `libmcmini.so` behaves exactly as in the classic case of
 * `TARGET_BRANCH`. The only difference is that the userspace threads appear
 * after a `dmtcp_restart` of a program in the middle of execution instead
 * directly launching the program. Userspace threads assume they are under the
 * control of the model checker.
 *
 */
enum libmcmini_mode {
  PRE_DMTCP_INIT,
  PRE_CHECKPOINT_THREAD,
  CHECKPOINT_THREAD,
  RECORD,
  PRE_CHECKPOINT,
  DMTCP_RESTART_INTO_BRANCH,
  DMTCP_RESTART_INTO_TEMPLATE,
  TARGET_TEMPLATE_AFTER_RESTART,
  TARGET_BRANCH_AFTER_RESTART,

  // Modes possible only when not using DMTCP
  // i.e. classic McMini.
  TARGET_TEMPLATE,
  TARGET_BRANCH,
};

/**
 * We use an `atomic_int` as opposed to `enum libmcmini_mode` directly
 * to ensure that writes from the checkpoint thread (which is responsible
 * for invoking our callback that in turn changes the mode) are visible to
 * all user-space threads.
 *
 * @note we CANNOT use a lock (at least easily) as e.g. during the
 * DMTCP_EVENT_RESTART event the user-space threads are blocked. This means that
 * using a lock to synchronize accesses to `libmcmini_mode` is dangerous: it's
 * possible that a user-space thread owned the lock at checkpoint-time (and
 * hence is also the owner at restart-time). If the checkpoint thread then tried
 * to acquire the lock, we'd have deadlock.
 */
extern volatile atomic_int libmcmini_mode;

bool is_in_restart_mode(void);
enum libmcmini_mode get_current_mode();
void set_current_mode(enum libmcmini_mode);

extern pthread_t ckpt_pthread_descriptor;
extern volatile atomic_bool libmcmini_has_recorded_checkpoint_thread;
bool is_checkpoint_thread(void);

typedef struct visible_object visible_object;
typedef struct rec_list rec_list;

extern sem_t dmtcp_restart_sem;
extern pthread_mutex_t rec_list_lock;
extern pthread_mutex_t dmtcp_list_lock;
extern rec_list *head_record_mode;
// The VIRTUAL tid of the checkpoint thread
// as it appeared while recording.

/// @brief Retrieves the stored state for the given object
/// @return a pointer to the node in the list formed by `head`,
/// or `NULL` if the object at address `addr` is not found
///
/// @note you must acquire `rec_list_lock` before calling this function
rec_list *find_object(void *addr, rec_list *);
rec_list *find_thread_record_mode(pthread_t);
rec_list *find_object_record_mode(void *addr);
rec_list *find_dmtcp_object(void *addr);
bool is_dmtcp_object(void *addr);

/// @brief Adds a new element to the list `head`.
///
/// @note you must acquire `rec_list_lock` before calling this function
rec_list *add_rec_entry(const visible_object *, rec_list **, rec_list **);
rec_list *add_rec_entry_record_mode(const visible_object *);
void print_rec_list(const rec_list *);
rec_list *add_dmctp_object(const visible_object *);

/**
 * @brief Notifies the template thread spawned during DMTCP_RESTART
 * that the current thread is now in a stable state and will
 * await further instruction from the scheduler process
 */
void notify_template_thread();

/**
 * @brief Either waits for the model checker `McMini` OR the template thread to
 * be awoken.
 */
void thread_wait_after_dmtcp_restart();

// Spawns a new process with all threads of this process duplicated.
pid_t multithreaded_fork(void);

#ifdef __cplusplus
}
#endif  // extern "C"
