#pragma once

#include <pthread.h>

#include "mcmini/lib/entry.h"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

void thread_await_scheduler(void);
void thread_wake_scheduler_and_wait(void);
void thread_awake_scheduler_for_thread_finish_transition(void);
void thread_handle_after_dmtcp_restart(void);
volatile runner_mailbox *thread_get_mailbox(void);

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr);
int mc_pthread_mutex_lock(pthread_mutex_t *mutex);
int mc_pthread_mutex_unlock(pthread_mutex_t *mutex);
int mc_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*routine)(void *), void *arg);
int mc_sem_post(sem_t *);
int mc_sem_wait(sem_t *);
int mc_pthread_join(pthread_t, void**);
int mc_sem_init(sem_t*, int, unsigned);
int mc_sem_post(sem_t*);
int mc_sem_wait(sem_t*);
int mc_sem_destroy(sem_t *sem);
unsigned mc_sleep(unsigned);

/*
  An `atexit()` handler is installed in libmcmini.so with this function.
  This ensures that if the main thread exits the model checker still maintains
  control.
*/
void mc_exit_main_thread_in_child(void);
MCMINI_NO_RETURN void mc_transparent_abort(void);
MCMINI_NO_RETURN void mc_transparent_exit(int status);
