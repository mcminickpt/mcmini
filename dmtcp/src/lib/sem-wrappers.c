#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <assert.h>

#include "mcmini/mcmini.h"

int mc_sem_init(sem_t *sem, int p, unsigned count) {
  // TODO: Does not handle interprocess semaphores
  assert(p == 0);

  switch (get_current_mode()) {
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      return libpthread_sem_init(sem, p, count);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *sem_record = find_object_record_mode(sem);
      if (sem_record == NULL) {
        visible_object vo = {.type = SEMAPHORE,
                             .location = sem,
                             .sem_state.status = SEM_UNINITIALIZED};
        sem_record = add_rec_entry_record_mode(&vo);
      }
      libpthread_mutex_unlock(&rec_list_lock);

      int rc = libpthread_sem_init(sem, p, count);
      if (rc == 0) {
        libpthread_mutex_lock(&rec_list_lock);
        rec_list *sem_record = find_object_record_mode(sem);
        assert(sem_record != NULL);
        visible_object vo = {.type = SEMAPHORE,
                             .location = sem,
                             .sem_state.status = SEM_INITIALIZED,
                             .sem_state.count = count};
        sem_record->vo = vo;
        libpthread_mutex_unlock(&rec_list_lock);
      }
      return rc;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART:
    case DMTCP_RESTART_INTO_BRANCH:
    case DMTCP_RESTART_INTO_TEMPLATE: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = SEM_INIT_TYPE;
      memcpy_v(mb->cnts, &sem, sizeof(sem));
      memcpy_v(mb->cnts + sizeof(sem), &count, sizeof(count));
      is_in_restart_mode() ? thread_handle_after_dmtcp_restart() : thread_wake_scheduler_and_wait();
      return libpthread_sem_init(sem, p, count);
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

int
mc_sem_post(sem_t *sem) {
switch (get_current_mode()) {
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      return libpthread_sem_post(sem);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *sem_record = find_object_record_mode(sem);
      if (sem_record == NULL) {
        // FIXME: We only change into record mode once the
        // checkpoint thread has
        int count = 0;
        sem_getvalue(sem, &count);
        visible_object vo = {.type = SEMAPHORE,
                             .location = sem,
                             .sem_state.status = SEM_INITIALIZED,
                             .sem_state.count = count};
        sem_record = add_rec_entry_record_mode(&vo);
      }
      libpthread_mutex_unlock(&rec_list_lock);
      int rc = libpthread_sem_post(sem);
      if (rc == 0) {  // Post succeeded
        libpthread_mutex_lock(&rec_list_lock);
        sem_record->vo.sem_state.count++;
        libpthread_mutex_unlock(&rec_list_lock);
      }
      return rc;
    }
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART:
    case DMTCP_RESTART_INTO_BRANCH:
    case DMTCP_RESTART_INTO_TEMPLATE: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = SEM_POST_TYPE;
      memcpy_v(mb->cnts, &sem, sizeof(sem));
      is_in_restart_mode() ? thread_handle_after_dmtcp_restart() : thread_wake_scheduler_and_wait();
      return libpthread_sem_post(sem);
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

int mc_sem_wait(sem_t *sem) {
  switch (get_current_mode()) {
    case PRE_DMTCP_INIT:
    case PRE_CHECKPOINT_THREAD: {
      return libpthread_sem_wait(sem);
    }
    case RECORD:
    case PRE_CHECKPOINT: {
      libpthread_mutex_lock(&rec_list_lock);
      rec_list *sem_record = find_object_record_mode(sem);
      if (sem_record == NULL) {
        int count = 0;
        sem_getvalue(sem, &count);
        visible_object vo = {.type = SEMAPHORE,
                             .location = sem,
                             .sem_state.status = SEM_INITIALIZED,
                             .sem_state.count = count};
        sem_record = add_rec_entry_record_mode(&vo);
      }
      libpthread_mutex_unlock(&rec_list_lock);

      struct timespec ts;
      while (1) {
        // Wait one second...
        clock_gettime(CLOCK_REALTIME, &ts); ts.tv_sec++;
        int rc = libpthread_sem_timedwait(sem, &ts);
        if (rc == 0) {  // Wait succeeded
          libpthread_mutex_lock(&rec_list_lock);
          sem_record->vo.sem_state.count--;
          libpthread_mutex_unlock(&rec_list_lock);
          return rc;
        } else if (rc == ETIMEDOUT) {  // If the lock failed.
          // Here, the user-space thread did not manage to wait on the
          // the lock. However, we do NOT want threads to block during
          // the recording phase to ensure that each user-space thread
          // can be put back under the control of the model checker.
          //
          // For those threads which have not managed to enter,
          // we want to ensure that they can eventually escape from this loop.
          // After the DMTCP_EVENT_RESTART event, other threads must notice that
          // the record phase has ended or else they could loop forever.
          if (is_in_restart_mode()) {
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
    case TARGET_BRANCH:
    case TARGET_BRANCH_AFTER_RESTART:
    case DMTCP_RESTART_INTO_BRANCH:
    case DMTCP_RESTART_INTO_TEMPLATE: {
      volatile runner_mailbox *mb = thread_get_mailbox();
      mb->type = SEM_WAIT_TYPE;
      memcpy_v(mb->cnts, &sem, sizeof(sem));
      is_in_restart_mode() ? thread_handle_after_dmtcp_restart() : thread_wake_scheduler_and_wait();
      return libpthread_sem_post(sem);
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
