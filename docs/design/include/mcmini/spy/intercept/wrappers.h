#pragma once

#include <pthread.h>

#include "mcmini/lib/entry.h"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

void thread_await_scheduler();
void thread_await_scheduler_for_thread_start_transition();
void thread_awake_scheduler_for_thread_finish_transition();
volatile runner_mailbox *thread_get_mailbox();

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr);

