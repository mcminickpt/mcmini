#include <assert.h>
#include <dmtcp.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mcmini/mcmini.h"

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
  return libpthread_mutex_init(mutex, attr);
}

int mc_pthread_mutex_lock(pthread_mutex_t *mutex) {
  int result = 0;
  int is_owner = 0;
  libpthread_mutex_lock(&rec_list_lock);
  rec_list *mutex_record = find_object_record_mode(mutex);
  if (mutex_record == NULL) {
    visible_object vo = {
            .type = MUTEX, .location = mutex, .mutex_state = UNINITIALIZED};
    mutex_record = add_rec_entry_record_mode(&vo);
  }
  libpthread_mutex_unlock(&rec_list_lock);
  if (libmcmini_mode == TARGET_BRANCH) {
    volatile runner_mailbox *mb = thread_get_mailbox();
    mb->type = MUTEX_LOCK_TYPE;
    memcpy_v(mb->cnts, &mutex, sizeof(mutex));
    thread_await_scheduler();
    return 0;
  }
  while (1) {
    int rc = pthread_mutex_trylock(mutex);
    if ( ! (rc == -1 && errno == EBUSY) ) { // If trylock succeeded
      libpthread_mutex_lock(&rec_list_lock);
      mutex_record->vo.mutex_state = LOCKED;
      libpthread_mutex_unlock(&rec_list_lock);
      result = rc;
      is_owner = 1;
      break;
    }
    if (libmcmini_mode == TARGET_BRANCH) { break; }
  }
  if (libmcmini_mode == TARGET_BRANCH) { // if a ckpt/restart happened in this wrapper
    libpthread_mutex_lock(&rec_list_lock);
    rec_list *mutex_during_record = find_object_record_mode(mutex);
    rec_list *mutex_after_restart = find_object(mutex, head_after_restart);
    libpthread_mutex_unlock(&rec_list_lock);
    // Test if we already owned this mutex prior to ckpt/restart
    if (memcmp(&mutex_during_record->vo, &mutex_after_restart, sizeof(visible_object))
        == 0) {
      // The model checker already knows we have the lock; nothing to do.
      // The model checker thinks were are between visible operations.
      return result;
    } else {
      // The model checker thought we don't have the lock.
      // We'll unlock it now, and make the model to be true in this target.
      if (is_owner) {
        pthread_mutex_unlock(mutex);
      }
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = MUTEX_LOCK_TYPE;
      memcpy_v(mb->cnts, &mutex, sizeof(mutex));
      thread_await_scheduler();
      return 0;
    }
  } else {
    return result;
  }
}

int mc_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  // switch (libmcmini_mode) {
  //   case RECORD: {
  //     libpthread_mutex_lock(&rec_list_lock);
  //     pending_operation *next_step = find_pending_op(pthread_self());
  //     rec_list *mutex_record = find_object(mutex);
  //     assert(mutex_record != NULL);
  //     assert(next_step != NULL);
  //     transition unlock = {.type = MUTEX_UNLOCK_TYPE,
  //                          .executor = pthread_self(),
  //                          .init = {.mut = &mutex_record->vo}};
  //     next_step->t = unlock;
  //     libpthread_mutex_unlock(&rec_list_lock);

  //     // It's possible that we checkpointed right before the call to
  //     // `dmtcp_disable_ckpt()`. In that case, the thread will resume
  //     // execution from the record portion; but since we want to
  //     // transfer control to the model checker now, we need to escape to the
  //     // next case.
  //     dmtcp_disable_ckpt();
  //     if (libmcmini_mode != RECORD) {
  //       int rc = libpthread_mutex_unlock(mutex);
  //       assert(rc == 0);
  //       libpthread_mutex_lock(&rec_list_lock);
  //       mutex_record->vo.mutex_state = UNLOCKED;
  //       next_step->t = invisible_operation_for_this_thread();
  //       libpthread_mutex_unlock(&rec_list_lock);
  //       dmtcp_enable_ckpt();
  //       return rc;
  //     } else {
  //       // Fallthrough to the model checker
  //       dmtcp_enable_ckpt();
  //     }
  //   }
  //   case TARGET_BRANCH:
  //   case TARGET_BRANCH_AFTER_RESTART: {
  //     volatile runner_mailbox *mb = thread_get_mailbox();
  //     mb->type = MUTEX_UNLOCK_TYPE;
  //     memcpy_v(mb->cnts, &mutex, sizeof(mutex));
  //     thread_await_scheduler();
  //     return 0;
  //   }
  //   default: {
  //     return libpthread_mutex_unlock(mutex);
  //   }
  // }
  return libpthread_mutex_unlock(mutex);
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
  mc_register_this_thread();
  struct mc_thread_routine_arg *unwrapped_arg = arg;

  switch (libmcmini_mode) {
    case TARGET_BRANCH: {
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

  switch (libmcmini_mode) {
    case TARGET_BRANCH:
      mc_exit_thread_in_child();
    default:
      break;
  }
  return return_value;
}

int mc_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                      void *(*routine)(void *), void *arg) {
  // TODO: Recording eventually will track thread operations
  switch (libmcmini_mode) {
    case TARGET_BRANCH: {
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
}

int mc_pthread_join(pthread_t t, void **rv) {
  switch (libmcmini_mode) {
    case TARGET_BRANCH: {
      memcpy_v(thread_get_mailbox()->cnts, &t, sizeof(pthread_t));
      thread_get_mailbox()->type = THREAD_JOIN_TYPE;
      thread_await_scheduler();
      return 0;
    }
    default: {
      return libpthread_pthread_join(t, rv);
    }
  }
}

unsigned mc_sleep(unsigned duration) {
  switch (libmcmini_mode) {
    case TARGET_BRANCH: {
      // Ignore actually putting this thread to sleep: it doesn't affect
      // correctness
      return 0;
    }
    default:
      return libc_sleep(duration);
  }
}
