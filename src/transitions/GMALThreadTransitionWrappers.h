#ifndef GMAL_GMALTHREADTRANSITIONWRAPPERS_H
#define GMAL_GMALTHREADTRANSITIONWRAPPERS_H

#include <pthread.h>
#include "../GMALShared.h"

int gmal_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
int gmal_pthread_join(pthread_t, void**);

/* Source program thread control */
void thread_await_gmal_scheduler();
void thread_await_gmal_scheduler_for_thread_start_transition();
void thread_awake_gmal_scheduler_for_thread_finish_transition();

#endif //GMAL_GMALTHREADTRANSITIONWRAPPERS_H
