#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mcmini/mcmini.h"

volatile runner_mailbox *thread_get_mailbox() {
  return ((volatile runner_mailbox*)(global_shm_start + THREAD_SHM_OFFSET)) + tid_self;
}

void
thread_await_scheduler()
{
  assert(tid_self != TID_INVALID);
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();
  mc_wake_scheduler(thread_mailbox);
  mc_wait_for_scheduler(thread_mailbox);
}

void
thread_await_scheduler_for_thread_start_transition()
{
  assert(tid_self != TID_INVALID);
  mc_wait_for_scheduler(thread_get_mailbox());
}

void
thread_awake_scheduler_for_thread_finish_transition()
{
  assert(tid_self != TID_INVALID);
  mc_wake_scheduler(thread_get_mailbox());
}

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  volatile runner_mailbox *mb = thread_get_mailbox();
  mb->type = MUTEX_INIT_TYPE;
  memcpy_v(mb->cnts, &mutex, sizeof(mutex));
  thread_await_scheduler();
  return 0;
}

int mc_pthread_mutex_lock(pthread_mutex_t *mutex) {
  volatile runner_mailbox *mb = thread_get_mailbox();
  mb->type = MUTEX_LOCK_TYPE;
  memcpy_v(mb->cnts, &mutex, sizeof(mutex));
  thread_await_scheduler();
  return 0;
}

int mc_pthread_mutex_unlock(pthread_mutex_t *mutex) {
  volatile runner_mailbox *mb = thread_get_mailbox();
  mb->type = MUTEX_UNLOCK_TYPE;
  memcpy_v(mb->cnts, &mutex, sizeof(mutex));
  thread_await_scheduler();
  return 0;
}