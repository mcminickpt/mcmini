#define _GNU_SOURCE
#include <assert.h>
#include <dmtcp.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mcmini/mcmini.h"

pthread_t main_thread;
MCMINI_THREAD_LOCAL runner_id_t tid_self = RID_INVALID;

runner_id_t mc_register_this_thread(void) {
  static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
  static runner_id_t tid_next = 0;

  libpthread_mutex_lock(&mut);
  tid_self = tid_next++;
  libpthread_mutex_unlock(&mut);
  return tid_self;
}

volatile runner_mailbox *thread_get_mailbox() {
  return &((volatile struct mcmini_shm_file *)(global_shm_start))
              ->mailboxes[tid_self];
}

void thread_await_scheduler(void) {
  assert(tid_self != RID_INVALID);
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();
  mc_wake_scheduler(thread_mailbox);

  errno = 0;
  int rc = mc_wait_for_scheduler(thread_mailbox);
  while (rc != 0 && errno == EINTR) {
    rc = mc_wait_for_scheduler(thread_mailbox);
  }
}

void thread_await_scheduler_for_thread_start_transition(void) {
  assert(tid_self != RID_INVALID);
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();

  errno = 0;
  int rc = mc_wait_for_scheduler(thread_mailbox);
  while (rc != 0 && errno == EINTR) {
    rc = mc_wait_for_scheduler(thread_mailbox);
  }
}

void thread_awake_scheduler_for_thread_finish_transition(void) {
  assert(tid_self != RID_INVALID);
  mc_wake_scheduler(thread_get_mailbox());
}

void thread_block_indefinitely() {
  while (1) {
    pause();
  }
}

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *attr) {
  // FIXME: Only handles NORMAL mutexes
  if (attr != NULL) {
    int type;
    pthread_mutexattr_gettype(attr, &type);
    assert(type == PTHREAD_MUTEX_NORMAL);
  }

  switch (get_current_mode()) {
    case PRE_DMTCP: {
      return libpthread_mutex_init(mutex, attr);
    }
    case RECORD: {
      break;
    }
    case DMTCP_RESTART: {
      notify_template_thread();
      // Explicit fallthrough intended here
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_INIT_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_await_scheduler();
      return libpthread_mutex_lock(mutex);
    }
    default: {
      // Wrapper functions should not be executing
      // inside the template! If we reach this point, it
      // means that this is a template process. This
      // method must have been directly called
      // erroneously.
      assert(0);
      break;
    }
  }
  // NOTE: This is subtle: at this point, the possible modes are
  // RECORD ***AND*** DMTCP_RESTART. The latter is possible if
  // checkpointing occurs anywhere AFTER the switch statement above.
  libpthread_mutex_lock(&rec_list_lock);
  rec_list *mutex_record = find_object_record_mode(mutex);
  if (mutex_record == NULL) {
    // FIXME: We assume that this is a normal mutex. For other mutex
    // types, we'd need to behave differently
    visible_object vo = {
        .type = MUTEX, .location = mutex, .mutex_state = UNINITIALIZED};
    mutex_record = add_rec_entry_record_mode(&vo);
  }
  libpthread_mutex_unlock(&rec_list_lock);

  int rc = libpthread_mutex_init(mutex, attr);
  if (rc == 0) {  // Init
    libpthread_mutex_lock(&rec_list_lock);
    mutex_record->vo.mutex_state = UNLOCKED;
    libpthread_mutex_unlock(&rec_list_lock);
  }
  return rc;
}

int mc_pthread_mutex_lock(pthread_mutex_t *mutex) {
  // On entry, there are several cases:
  //
  // 1. The thread was executing before dmtcp has had a chance
  // to notify us of the DMTCP_EVENT_INIT. Here we forward
  // the call to `libpthread.so` directly to avoid recording
  // the system calls dmtcp makes in order to set up
  // restart (i.e. prior to invoking our callback)
  //
  // 2. The thread was executing invisible operations during
  // restart and has since reached the next visible operation.
  // In this case, before shifting into model checking mode, we
  // need to notify the template thread that the thread has
  // completed all its recording.
  //
  // 3. The thread was executing a visible operation and has since
  // reached its next visible operation as a consequence of
  // control by the scheduler. Direct transfer to model checking
  // mode suffices.
  //
  // 4. The thread is executing in RECORD mode and should do state
  // tracking.
  switch (get_current_mode()) {
    case PRE_DMTCP: {
      return libpthread_mutex_lock(mutex);
    }
    case RECORD: {

      // NOTE: This is subtle: at this point, the possible modes are
      // RECORD ***AND*** DMTCP_RESTART. The latter is possible if
      // checkpointing occurs anywhere AFTER the switch statement above.
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *mutex_record = find_object_record_mode(mutex);
      if (mutex_record == NULL) {
        visible_object vo = {
            .type = MUTEX, .location = mutex, .mutex_state = UNINITIALIZED};
        mutex_record = add_rec_entry_record_mode(&vo);
      }
      libpthread_mutex_unlock(&rec_list_lock);

      int result = 0;
      struct timespec time = {.tv_sec = 2};
      while (1) {
        int rc = libpthread_mutex_timedlock(mutex, &time);
        if (rc == 0) {  // Lock succeeded
          libpthread_mutex_lock(&rec_list_lock);
          mutex_record->vo.mutex_state = LOCKED;
          libpthread_mutex_unlock(&rec_list_lock);
          result = rc;
          break;
        } else if (rc == ETIMEDOUT) {  // If the lock failed.
          // Here, the user-space thread did not manage to acquire
          // the lock. However, we do NOT want threads to block during
          // the recording phase to ensure that each user-space thread
          // can be put back under the control of the model checker.
          //
          // For those threads which have not managed to acquire the lock,
          // we want to ensure that they can eventually escape from this loop.
          // After the DMTCP_EVENT_RESTART event, exactly one thread will
          // successfully acquire the lock. Other threads must notice that
          // the record phase has ended or else they could loop forever.
          if (get_current_mode() == DMTCP_RESTART) {
            break;
          }
        } else if (rc != 0 && rc != ETIMEDOUT) {
          // A "true" error: something went wrong with locking
          // and we pass this on to the end user
          return rc;
        }
      }
      // The thread may or may not be the owner of the lock.
      //
      // 1. If the thread owns the lock, it will have recorded
      // that in the loop above and the scheduler will be informed of it.
      //
      // 2. If the thread does not own the lock, nothing need be done since
      // the thread which managed to acquire the lock will have informed the
      // scheduler.
      //
      // In both cases, the thread must be placed back under the control
      // of the scheduler and inform the template thread that it
      // is now in a consistent state.
      if (get_current_mode() == DMTCP_RESTART) {
        notify_template_thread();
        break;
      } else {
        return result;
      }
    }
    case DMTCP_RESTART: {
      notify_template_thread();
      break;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      break;
    }
    default: {
      // Wrapper functions should not be executing
      // inside the template! If we reach this point, it
      // means that this is a template process. This
      // method must have been directly called
      // erroneously.
      assert(0);
      break;
    }
  }
  volatile runner_mailbox *mb = thread_get_mailbox();
  mb->type = MUTEX_LOCK_TYPE;
  memcpy_v(mb->cnts, &mutex, sizeof(mutex));
  thread_await_scheduler();
  return libpthread_mutex_lock(mutex);
}

int mc_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  switch (get_current_mode()) {
    case PRE_DMTCP: {
      return libpthread_mutex_unlock(mutex);
    }
    case RECORD: {
      break;
    }
    case DMTCP_RESTART: {
      notify_template_thread();
      // Explicit fallthrough intended
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_UNLOCK_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_await_scheduler();
      return libpthread_mutex_lock(mutex);
    }
    default: {
      // Wrapper functions should not be executing
      // inside the template! If we reach this point, it
      // means that this is a template process. This
      // method must have been directly called
      // erroneously.
      assert(0);
      break;
    }
  }
  // NOTE: This is subtle: at this point, the possible modes are
  // RECORD ***AND*** DMTCP_RESTART. The latter is possible if
  // checkpointing occurs anywhere AFTER the switch statement above.
  libpthread_mutex_lock(&rec_list_lock);
  rec_list *mutex_record = find_object_record_mode(mutex);
  if (mutex_record == NULL) {
    // FIXME: We assume that this is a normal mutex. For other mutex
    // types, we'd need to behave differently
    fprintf(
        stderr,
        "Undefined behavior: attempting to unlock an uninitialized mutex %p",
        mutex);
    assert(0);
  }
  libpthread_mutex_unlock(&rec_list_lock);

  int rc = libpthread_mutex_unlock(mutex);
  if (rc == 0) {  // Unlock succeeded
    libpthread_mutex_lock(&rec_list_lock);
    mutex_record->vo.mutex_state = UNLOCKED;
    libpthread_mutex_unlock(&rec_list_lock);
  }
  return rc;
}

void mc_exit_thread_in_child(void) {
  thread_get_mailbox()->type = THREAD_EXIT_TYPE;
  thread_await_scheduler();
  thread_awake_scheduler_for_thread_finish_transition();
}

void mc_exit_main_thread_in_child(void) {
  if (tid_self != RID_MAIN_THREAD) libc_abort();
  // IMPORTANT: This is NOT a typo!
  // 1. `thread_await_scheduler()` is called when the
  // main thread is known to be _alive_ to the model
  // 2. `thread_awake_scheduler_for_thread_finish_transition()`
  // is called to simulate the thread having "exited"
  // 3. `thread_block_indefinitely()` ensures that the
  // main thread never actually exits
  //
  // However, unlike a normal thread exiting, we want to ensure that
  // the process doesn't terminate; hence, we prevent the main thread
  // from ever escaping this function.
  thread_get_mailbox()->type = THREAD_EXIT_TYPE;
  thread_await_scheduler();

  thread_get_mailbox()->type = THREAD_EXIT_TYPE;
  thread_awake_scheduler_for_thread_finish_transition();
  thread_block_indefinitely();
}

MCMINI_NO_RETURN void mc_transparent_exit(int status) {
  volatile runner_mailbox *mb = thread_get_mailbox();
  mb->type = PROCESS_EXIT_TYPE;
  memcpy_v(mb->cnts, &status, sizeof(status));
  thread_await_scheduler();
  libc_exit(status);
}

MCMINI_NO_RETURN void mc_transparent_abort(void) {
  thread_await_scheduler();
  libc_abort();
}

struct mc_thread_routine_arg {
  void *arg;
  thread_routine routine;
  sem_t mc_pthread_create_binary_sem;
};

void *mc_thread_routine_wrapper(void *arg) {
  // FIXME: This needs to be adjusted to the new cases
  // (the DMTCP_RESTART case in particular)
  mc_register_this_thread();
  struct mc_thread_routine_arg *unwrapped_arg = arg;

  switch (get_current_mode()) {
    case DMTCP_AFTER_RESTART: {
      notify_template_thread();
      // Explicit fallthrough here
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      libpthread_sem_post(&unwrapped_arg->mc_pthread_create_binary_sem);
      // Simulates THREAD_START for this thread NOTE:
      // We don't need to write into shared memory here. The
      // scheduler already knows how to handle the case
      // of thread creation
      thread_await_scheduler_for_thread_start_transition();
    }
    default:
      break;
  }

  void *return_value = unwrapped_arg->routine(unwrapped_arg->arg);
  free(arg);

  switch (get_current_mode()) {
    case TARGET_BRANCH:
      mc_exit_thread_in_child();
    default:
      break;
  }
  return return_value;
}

int mc_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                      void *(*routine)(void *), void *arg) {
  // FIXME: Needs implementation with DMTCP_RESTART etc.
  static bool main_thread_known = false;
  // static pthread_once_t main_thread_recorded = PTHREAD_ONCE_INIT;
  // pthread_once()
  if (!main_thread_known) {
    main_thread = pthread_self();
    libpthread_mutex_lock(&rec_list_lock);
    rec_list *thread_record = find_thread_record_mode(main_thread);
    assert(thread_record == NULL);
    // visible_object vo = {
    //         .type = MUTEX, .location = NULL, .thread_state = main_thread};
    // thread_record = add_rec_entry_record_mode(&vo);
    libpthread_mutex_unlock(&rec_list_lock);
    main_thread_known = true;
  }

  switch (get_current_mode()) {
    case PRE_DMTCP: {
      // NOTE: We're assuming that only a single checkpoint
      // thread enters model checking mode and that this thread
      // is more of less "boring" (i.e. it doesn't call pthread functions)
      return libpthread_pthread_create(thread, attr, routine, arg);
    }
    case RECORD: {
      break;
    }
    case DMTCP_RESTART: {
      notify_template_thread();
      // Explicity fallthrough here
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      // TODO: add support for thread attributes
      struct mc_thread_routine_arg *libmcmini_controlled_thread_arg =
          malloc(sizeof(struct mc_thread_routine_arg));
      libmcmini_controlled_thread_arg->arg = arg;
      libmcmini_controlled_thread_arg->routine = routine;
      libpthread_sem_init(
          &libmcmini_controlled_thread_arg->mc_pthread_create_binary_sem, 0, 0);

      errno = 0;
      const int return_value =
          libpthread_pthread_create(thread, attr, &mc_thread_routine_wrapper,
                                    libmcmini_controlled_thread_arg);

      // IMPORTANT: We need to ensure that the thread that is
      // created has been assigned an; otherwise, there is a race condition
      // in which two thread creates in the child might
      // not be scheduled to run until *two* steps of the scheduler
      libpthread_sem_wait(
          &libmcmini_controlled_thread_arg->mc_pthread_create_binary_sem);

      memcpy_v(thread_get_mailbox()->cnts, thread, sizeof(pthread_t));
      thread_get_mailbox()->type = THREAD_CREATE_TYPE;
      thread_await_scheduler();
      return return_value;
    }
    default: {
      struct mc_thread_routine_arg *record_arg =
          malloc(sizeof(struct mc_thread_routine_arg));
      record_arg->arg = arg;
      record_arg->routine = routine;
      return libpthread_pthread_create(thread, attr, &mc_thread_routine_wrapper,
                                       record_arg);
    }
  }
  return 0;
}

int mc_pthread_join(pthread_t t, void **rv) {
  // FIXME: This code should look very similar to
  // the code in `mc_pthread_mutex_lock`. Here we
  // can use the `pthread_timedjoin_np` GNU extension
  switch (get_current_mode()) {
    case PRE_DMTCP: {
      return libpthread_pthread_join(t, rv);
    }
    case RECORD: {
      break;
    }
    case DMTCP_RESTART: {
      notify_template_thread();
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      break;
    }
    default: {
      assert(0);
      break;
    }
  }

  memcpy_v(thread_get_mailbox()->cnts, &t, sizeof(pthread_t));
  thread_get_mailbox()->type = THREAD_JOIN_TYPE;
  thread_await_scheduler();
  return 0;
}

unsigned mc_sleep(unsigned duration) {
  switch (get_current_mode()) {
    case DMTCP_RESTART:
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      // Ignore actually putting this thread to sleep:
      // it doesn't affect correctness neither for model
      // checking nor for state regenetation.
      return 0;
    }
    default:
      return libc_sleep(duration);
  }
}
