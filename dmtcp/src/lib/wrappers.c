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
#include "mcmini/common/exit.h"

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

void thread_wake_scheduler_and_wait(void) {
  assert(tid_self != RID_INVALID);
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();
  errno = 0;
  int wake_rc = mc_wake_scheduler(thread_mailbox);
  assert(wake_rc == 0);
  assert(errno == 0);

  errno = 0;
  int rc = mc_wait_for_scheduler(thread_mailbox);
  while (rc != 0 && errno == EINTR) {
    rc = mc_wait_for_scheduler(thread_mailbox);
  }
  assert(errno != EINVAL);
}

void thread_await_scheduler(void) {
  assert(tid_self != RID_INVALID);
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();

  errno = 0;
  int rc = mc_wait_for_scheduler(thread_mailbox);
  while (rc != 0 && errno == EINTR) {
    rc = mc_wait_for_scheduler(thread_mailbox);
  }
  assert(errno != EINVAL);
}

void thread_awake_scheduler_for_thread_finish_transition(void) {
  assert(tid_self != RID_INVALID);
  mc_wake_scheduler(thread_get_mailbox());
}

void thread_block_indefinitely(void) {
  while (1) {
    pause();
  }
}

void thread_handle_after_dmtcp_restart(void);

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *attr) {
  // FIXME: Only handles NORMAL mutexes
  if (attr != NULL) {
    int type;
    pthread_mutexattr_gettype(attr, &type);
    assert(type == PTHREAD_MUTEX_NORMAL);
  }

  switch (get_current_mode()) {
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      return libpthread_mutex_init(mutex, attr);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      // NOTE: This is subtle: at this point, the possible modes are
      // RECORD ***AND*** DMTCP_RESTART. The latter is possible if
      // checkpointing occurs anywhere AFTER the switch statement above.
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *mutex_record = find_object_record_mode(mutex);
      if (mutex_record == NULL) {
        // FIXME: We assume that this is a normal mutex. For other mutex
        // types, we'd need to behave differently
        visible_object vo = {
            .type = MUTEX, .location = mutex, .mut_state = UNINITIALIZED};
        mutex_record = add_rec_entry_record_mode(&vo);
      }
      libpthread_mutex_unlock(&rec_list_lock);

      int rc = libpthread_mutex_init(mutex, attr);
      if (rc == 0) {  // Init
        libpthread_mutex_lock(&rec_list_lock);
        mutex_record->vo.mut_state = UNLOCKED;
        libpthread_mutex_unlock(&rec_list_lock);
      }
      return rc;
    }
    case DMTCP_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_INIT_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_handle_after_dmtcp_restart();
      return libpthread_mutex_init(mutex, attr);
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_INIT_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_wake_scheduler_and_wait();
      return libpthread_mutex_init(mutex, attr);
    }
    default: {
      // Wrapper functions should not be executing
      // inside the template! If we reach this point, it
      // means that this is a template process. This
      // method must have been directly called
      // erroneously.
      libc_abort();
    }
  }
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
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      return libpthread_mutex_lock(mutex);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      // NOTE: This is subtle: at this point, the possible modes are
      // RECORD ***AND*** DMTCP_RESTART. The latter is possible if
      // checkpointing occurs anywhere AFTER the switch statement above.
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *mutex_record = find_object_record_mode(mutex);
      if (mutex_record == NULL) {
        visible_object vo = {
            .type = MUTEX, .location = mutex, .mut_state = UNINITIALIZED};
        mutex_record = add_rec_entry_record_mode(&vo);
      }
      libpthread_mutex_unlock(&rec_list_lock);

      struct timespec time = {.tv_sec = 2};
      while (1) {
        int rc = libpthread_mutex_timedlock(mutex, &time);
        if (rc == 0) {  // Lock succeeded
          libpthread_mutex_lock(&rec_list_lock);
          mutex_record->vo.mut_state = LOCKED;
          libpthread_mutex_unlock(&rec_list_lock);
          return rc;
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
      // Explicit fallthrough
    }
    case DMTCP_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_LOCK_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_handle_after_dmtcp_restart();
      return libpthread_mutex_lock(mutex);
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_LOCK_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_wake_scheduler_and_wait();
      return libpthread_mutex_lock(mutex);
    }
    default: {
      // Wrapper functions should not be executing
      // inside the template! If we reach this point, it
      // means that this is a template process. This
      // method must have been directly called
      // erroneously.
      libc_abort();
    }
  }
}

int mc_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  switch (get_current_mode()) {
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      return libpthread_mutex_unlock(mutex);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *mutex_record = find_object_record_mode(mutex);
      if (mutex_record == NULL) {
        // FIXME: We assume that this is a normal mutex. For other mutex
        // types, we'd need to behave differently
        fprintf(stderr,
                "Undefined behavior: attempting to unlock an uninitialized "
                "mutex %p",
                mutex);
        libc_abort();
      }
      libpthread_mutex_unlock(&rec_list_lock);
      int rc = libpthread_mutex_unlock(mutex);
      if (rc == 0) {  // Unlock succeeded
        libpthread_mutex_lock(&rec_list_lock);
        mutex_record->vo.mut_state = UNLOCKED;
        libpthread_mutex_unlock(&rec_list_lock);
      }
      return rc;
    }
    case DMTCP_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_UNLOCK_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_handle_after_dmtcp_restart();
      return libpthread_mutex_unlock(mutex);
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_UNLOCK_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_wake_scheduler_and_wait();
      return libpthread_mutex_unlock(mutex);
    }
    default: {
      // Wrapper functions should not be executing
      // inside the template! If we reach this point, it
      // means that this is a template process. This
      // method must have been directly called
      // erroneously.
      libc_abort();
    }
  }
}

void mc_exit_thread_in_child(void) {
  thread_get_mailbox()->type = THREAD_EXIT_TYPE;
  thread_wake_scheduler_and_wait();
  thread_awake_scheduler_for_thread_finish_transition();
}

void mc_exit_main_thread_in_child(void) {
  if (tid_self != RID_MAIN_THREAD) libc_abort();
  // IMPORTANT: This is NOT a typo!
  // 1. `thread_wake_scheduler_and_wait()` is called when the
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
  thread_wake_scheduler_and_wait();

  thread_get_mailbox()->type = THREAD_EXIT_TYPE;
  thread_awake_scheduler_for_thread_finish_transition();
  thread_block_indefinitely();
}

MCMINI_NO_RETURN void mc_transparent_exit(int status) {
  volatile runner_mailbox *mb = thread_get_mailbox();
  mb->type = PROCESS_EXIT_TYPE;
  memcpy_v(mb->cnts, &status, sizeof(status));
  thread_wake_scheduler_and_wait();
  libc_exit(status);
}

MCMINI_NO_RETURN void mc_transparent_abort(void) {
  thread_wake_scheduler_and_wait();
  libc_abort();
}

struct mc_thread_routine_arg {
  void *arg;
  thread_routine routine;
  sem_t mc_pthread_create_binary_sem;
};

void *mc_thread_routine_wrapper(void *arg) {
  runner_id_t rid = mc_register_this_thread();
  struct mc_thread_routine_arg *unwrapped_arg = arg;
  switch (get_current_mode()) {
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      fprintf(stderr,
      "In `PRE_DMTCP_INIT` mode, `mc_pthread_create` always directly calls DMTCP."
      "Reaching this point means that the McMini wrapper would be an error.\n");
      libc_abort();
    }
    case RECORD:
    case PRE_CHECKPOINT:
    case DMTCP_RESTART: {
      // If we've noticed we're executing after a `DMTCP_EVENT_RESTART`, we
      // simply let the thread continue executing until one of two things
      // happens:
      //
      // 1. The thread routine itself calls another wrapper function (i.e.
      // visible operation)
      // 2. The thread routine exits before calling another wrapper function.
      //
      // The same applies to RECORD mode: we simply do the recording and let the
      // thread routine call the next wrapper function.
      pthread_t this_thread = pthread_self();
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *thread_record = find_thread_record_mode(this_thread);
      assert(thread_record == NULL);
      visible_object vo = {.type = THREAD,
                           .location = NULL,
                           .thrd_state.pthread_desc = this_thread,
                           .thrd_state.status = ALIVE,
                           .thrd_state.id = rid};
      thread_record = add_rec_entry_record_mode(&vo);
      libpthread_mutex_unlock(&rec_list_lock);
      libpthread_sem_post(&unwrapped_arg->mc_pthread_create_binary_sem);
      break;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      libpthread_sem_post(&unwrapped_arg->mc_pthread_create_binary_sem);
      // Simulates THREAD_START for this thread NOTE:
      // We don't need to write into shared memory here. The
      // scheduler already knows how to handle the case
      // of thread creation
      thread_await_scheduler();
      break;
    }
    default: {
      libc_abort();
    }
  }

  void *rv = unwrapped_arg->routine(unwrapped_arg->arg);
  free(arg);

  switch (get_current_mode()) {
    case RECORD:
    case PRE_CHECKPOINT: {
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *thread_record = find_thread_record_mode(pthread_self());
      assert(thread_record != NULL);
      thread_record->vo.thrd_state.status = EXITED;
      libpthread_mutex_unlock(&rec_list_lock);
      return rv;
    }
    case DMTCP_RESTART: {
      thread_get_mailbox()->type = THREAD_EXIT_TYPE;
      thread_handle_after_dmtcp_restart();
      thread_awake_scheduler_for_thread_finish_transition();
      break;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      mc_exit_thread_in_child();
      break;
    }
    default: {
      libc_abort();
    }
  }
  return rv;
}

void record_main_thread(void) {
  pthread_t main_thread = pthread_self();
  runner_id_t main_tid = mc_register_this_thread();
  assert(main_tid == 0);
  libpthread_mutex_lock(&rec_list_lock);
  rec_list *thread_record = find_thread_record_mode(main_thread);
  assert(thread_record == NULL);
  visible_object vo = {.type = THREAD,
                       .location = NULL,
                       .thrd_state.id = main_tid,
                       .thrd_state.pthread_desc = main_thread,
                       .thrd_state.status = ALIVE};
  thread_record = add_rec_entry_record_mode(&vo);
  libpthread_mutex_unlock(&rec_list_lock);

  // Recording the presence of the main thread means that the main
  // thread has made its first call to `pthread_create`. The assumption
  // is that DMTCP (executing in the main thread before the `main` routine)
  // makes the first call to `pthread_create` to create the checkpoint thread.
  // Since the checkpoint thread is about to be created, it is safe to begin
  // recording.
  atomic_store(&libmcmini_mode, RECORD);
}

int mc_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                      void *(*routine)(void *), void *arg) {
  // NOTE: We're assuming that DMTCP creates only the checkpoint thread
  // immediately after sending the `DMTCP_EVENT_INIT` to `libmcmini.so`
  // and creates no other threads during execution
  static pthread_once_t main_thread_once = PTHREAD_ONCE_INIT;

  // TODO: Reduce code duplication here!
  switch (get_current_mode()) {
    case PRE_DMTCP_INIT: {
      // This case implies that DMTCP attempted to create
      // a thread BEFORE the `DMTCP_EVENT_INIT` was delivered
      // to `libmcmini`'s callback.
      // NOTE: Explicit fallthrough intended
      assert(0);
    }
    case PRE_CHECKPOINT_THREAD: {
      pthread_once(&main_thread_once, &record_main_thread);
      return libdmtcp_pthread_create(thread, attr, routine, arg);
    }
    case RECORD:
    case PRE_CHECKPOINT:
    case DMTCP_RESTART: {
      // TODO: add support for thread attributes
      struct mc_thread_routine_arg *libmcmini_controlled_thread_arg =
          malloc(sizeof(struct mc_thread_routine_arg));
      libmcmini_controlled_thread_arg->arg = arg;
      libmcmini_controlled_thread_arg->routine = routine;
      // TODO: Handle the errors that can occur when
      // pthread_create is called. They are unlikely to
      // occur in practice, but should be handled
      libpthread_sem_init(
          &libmcmini_controlled_thread_arg->mc_pthread_create_binary_sem, 0,
          0);
      const int rc =
          libdmtcp_pthread_create(thread, attr, &mc_thread_routine_wrapper,
                                    libmcmini_controlled_thread_arg);
      // IMPORTANT: We need to ensure that the child thread is recorded
      // before exiting; otherwise there are potential race conditions
      // in the record code (e.g. with `pthread_join()`) which expect the
      // child thread to have been recorded when it may not have been yet.
      libpthread_sem_wait(
          &libmcmini_controlled_thread_arg->mc_pthread_create_binary_sem);
      return rc;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      // TODO: add support for thread attributes
      struct mc_thread_routine_arg *libmcmini_controlled_thread_arg =
          malloc(sizeof(struct mc_thread_routine_arg));
      libmcmini_controlled_thread_arg->arg = arg;
      libmcmini_controlled_thread_arg->routine = routine;
      libpthread_sem_init(
          &libmcmini_controlled_thread_arg->mc_pthread_create_binary_sem, 0,
          0);

      // NOTE: This code is reached when the model checker has control.
      // Since this process represents an(emphemeral) branch of state space,
      // unless we later want to checkpoint this branch, we don't need to
      // inform DMTCP of these new threads. Indeed, in classic model checking
      // mode, `libdmtcp.so` is not even loaded (so we'd have to check first anyway).
      // Calling `libpthread_pthread_create` simplifies all this.
      const int rv =
          libpthread_pthread_create(thread, attr, &mc_thread_routine_wrapper,
                                    libmcmini_controlled_thread_arg);

      // IMPORTANT: We need to ensure that the thread that is
      // created has been assigned an id; otherwise, there is a race condition
      // in which two thread creates in the child might
      // not be scheduled to run until *two* steps of the scheduler
      libpthread_sem_wait(
          &libmcmini_controlled_thread_arg->mc_pthread_create_binary_sem);

      memcpy_v(thread_get_mailbox()->cnts, thread, sizeof(pthread_t));
      thread_get_mailbox()->type = THREAD_CREATE_TYPE;
      thread_wake_scheduler_and_wait();
      return rv;
    }
    default: {
      libc_abort();
    }
  }
}

int mc_pthread_join(pthread_t t, void **rv) {
  switch (get_current_mode()) {
    case PRE_DMTCP_INIT: {
      // This case implies that DMTCP attempted to join
      // a thread BEFORE the `DMTCP_EVENT_INIT` was delivered
      // to `libmcmini`'s callback
      // NOTE: Explicit fallthrough intended
      assert(0);
    }
    case PRE_CHECKPOINT_THREAD: {
      return libdmtcp_pthread_join(t, rv);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      // NOTE: This is subtle: at this point, the possible modes are
      // RECORD ***AND*** DMTCP_RESTART. The latter is possible if
      // checkpointing occurs anywhere AFTER the switch statement above.
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *thread_record = find_thread_record_mode(t);
      assert(thread_record != NULL);
      libpthread_mutex_unlock(&rec_list_lock);

      struct timespec time = {.tv_sec = 2, .tv_nsec = 0};
      while (1) {
        int rc = pthread_timedjoin_np(t, rv, &time);
        if (rc == 0) {  // Join succeeded
          libpthread_mutex_lock(&rec_list_lock);
          thread_record->vo.thrd_state.status = EXITED;
          libpthread_mutex_unlock(&rec_list_lock);
          return rc;
        } else if (rc == ETIMEDOUT) {
          // If the join failed.
          // Here, the user-space thread did not manage to join on
          // the thread. However, we do NOT want threads to block during
          // the recording phase to ensure that each user-space thread
          // can be put back under the control of the model checker.
          //
          // For those threads which have not managed to join on the thread,
          // we want to ensure that they can eventually escape from this loop.
          // After the DMTCP_EVENT_RESTART event, exactly one thread will
          // successfully acquire the lock. Other threads must notice that
          // the record phase has ended or else they would loop forever.
          if (get_current_mode() == DMTCP_RESTART) {
            break;
          }
        } else if (rc != 0 && rc != ETIMEDOUT) {
          // A "true" error: something went wrong with locking
          // and we pass this on to the end user
          return rc;
        }
      }
    }
    // Explicit fallthrough here
    case DMTCP_RESTART: {
      memcpy_v(thread_get_mailbox()->cnts, &t, sizeof(pthread_t));
      thread_get_mailbox()->type = THREAD_JOIN_TYPE;
      thread_handle_after_dmtcp_restart();
      return 0;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART: {
      memcpy_v(thread_get_mailbox()->cnts, &t, sizeof(pthread_t));
      thread_get_mailbox()->type = THREAD_JOIN_TYPE;
      thread_wake_scheduler_and_wait();
      return 0;
    }
    default: {
      libc_abort();
    }
  }
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
