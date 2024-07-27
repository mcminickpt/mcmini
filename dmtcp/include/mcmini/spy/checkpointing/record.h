#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include "mcmini/spy/checkpointing/objects.h"
#include "mcmini/spy/checkpointing/transitions.h"

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
 * TARGET_TEMPLATE:
 *   In this mode, the `mcmini` `fork()/exec()`-ed a new process (the template)
 *   running the target program with `libmcmini.so` preloaded. In this mode,
 *   `mcmini` expects to be able to repeatedly signal this process when the model
 *   checker wants to explore a new branch of the state space. `libmcmini.so`
 *   enters an infinite loop waiting for the signal to `fork()`. Wrapper functions simply
 *   forward calls to the next available function found by `dlsym(3)` using `RTLD_NEXT`.
 *
 * TARGET_BRANCH:
 *   In this mode, `mcmini` has signaled the template process and expects a new process
 *   to be created. In this process, the wrapper functions in `libmcmini.so` will now
 *   behave as if under the control of the model checker in `mcmini`.
 *
 * PRE_DMTCP:
 *   In this mode, the `mcmini` process has `exec()`-ed into `dmtcp_launch` with `libmcminio.so`
 *   as a DMTCP plugin. Here, DMTCP will preload mcmini. Prior to DMTCP alerting us with the
 *   `DMTCP_EVENT_INIT`, wrapper functions simply forward calls to the next available
 *   function found by `dlsym(3)` using `RTLD_NEXT`.
 *
 * RECORD:
 *   In this mode, `libmcmini.so` performs a light-weight recording of the primitives it
 *   encounters and keeps track of their state. Only after doing so do wrapper functions
 *   forward calls to the next available function found by `dlsym(3)` using `RTLD_NEXT`.
 *
 * PRE_CHECKPOINT:
 *   In this mode, `libmcmini.so` has been alerted about the end of the recording session.
 *   AWrapper functions again forward their calls to the next available function.
 *
 * TARGET_TEMPLATE_AFTER_RESTART:
 *   In this mode, `libmcmini.so` has been restored by `dmtcp_launch` from a checkpoint image.
 *   Here, `libmcmini.so` will first attempt to communicate with `mcmini` and transfer over
 *   the state information recorded during the RECORD phase. After transfer has completed,
 *   `libmcmini.so` will behave the same as `TARGET_TEMPLATE`.
 *
 * TARGET_BRANCH_AFTER_RESTART:
 *   In this mode, `mcmini` has signaled the restarted template process and expects a new process
 *   to be created. Here, `libmcmini.so` must now have wrapper functions behave as if under
 *   the control of the model checker as with `TARGET_BRANCH`.
 */
enum libmcmini_mode {
  TARGET_TEMPLATE,
  TARGET_BRANCH,
  PRE_DMTCP,
  RECORD,
  PRE_CHECKPOINT,
  TARGET_TEMPLATE_AFTER_RESTART,
  TARGET_BRANCH_AFTER_RESTART,
};

extern enum libmcmini_mode libmcmini_mode;

typedef struct rec_list {
  visible_object vo;
  struct rec_list *next;
} rec_list;

extern rec_list *head;
extern rec_list *current;
extern pthread_mutex_t rec_list_lock;

typedef struct pending_operation {
  transition t;
  struct pending_operation *next;
} pending_operation;

extern pending_operation *head_op;
extern pending_operation *current_op;
extern pthread_mutex_t pending_op_lock;

/// @brief Retrieves the stored state for the given object
/// @return a pointer to the node in the list formed by `head`,
/// or `NULL` if the object at address `addr` is not found
///
/// @note you must acquire `rec_list_lock` before calling this function
rec_list *find_object(void *addr);
pending_operation *find_pending_op(pthread_t);

/// @brief Adds a new element to the list `head`.
///
/// @note you must acquire `rec_list_lock` before calling this function
rec_list *add_rec_entry(const visible_object *);
pending_operation *add_pending_op(const transition *);

#ifdef __cplusplus
}
#endif  // extern "C"
