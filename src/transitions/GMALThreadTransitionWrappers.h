#ifndef GMAL_GMALTHREADTRANSITIONWRAPPERS_H
#define GMAL_GMALTHREADTRANSITIONWRAPPERS_H

#include <pthread.h>
#include <typeinfo>
#include "../GMALShared.h"


int gmal_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
int gmal_pthread_join(pthread_t, void**);
void gmal_exit_main_thread();

/* Source program thread control */
template<typename SharedMemoryData> void
thread_post_visible_operation_hit(const std::type_info &type, SharedMemoryData * shmData);
void thread_post_visible_operation_hit(const std::type_info &type);

void thread_await_gmal_scheduler();
void thread_await_gmal_scheduler_for_thread_start_transition();
void thread_awake_gmal_scheduler_for_thread_finish_transition();

#endif //GMAL_GMALTHREADTRANSITIONWRAPPERS_H
