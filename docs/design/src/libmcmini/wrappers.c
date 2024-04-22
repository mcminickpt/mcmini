#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mcmini/mcmini.h"

volatile runner_mailbox *thread_get_mailbox() {
  return ((volatile runner_mailbox*)(shm_start)) + tid_self;
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
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();
  mc_wait_for_scheduler(thread_mailbox);
}

void
thread_awake_scheduler_for_thread_finish_transition()
{
  assert(tid_self != TID_INVALID);
  volatile runner_mailbox *thread_mailbox = thread_get_mailbox();
  mc_wake_scheduler(thread_mailbox);
}

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  // TODO: write into the shm region enough information
  // to determine what just happened on the model side

  volatile runner_mailbox *mb = thread_get_mailbox();
  memcpy((void*)mb->cnts, &mutex, sizeof(pthread_mutex_t));

  // The coordinator first assumes data is written as follows:
  // transition id followed by payload.
  //
  // TODO: There's no system in place to synchronize transition ids
  // with the registration on the model side. This is a dynamic process
  // (new transition can be added at runtime)
  // For now, it suffices to assign a fixed value and just assume it
  // corresponds on the model side

  thread_await_scheduler();
  return 0;
}