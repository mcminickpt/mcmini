//
// Created by parallels on 4/12/22.
//

#include "GMALThreadTransitionWrappers.h"
#include "../GMAL.h"

struct gmal_thread_routine_arg {
    void *arg;
    thread_routine routine;
};


void *
gmal_thread_routine_wrapper(void * arg)
{
    tid_self = programState.createNewThread();
    sem_post(&gmal_pthread_create_binary_sem);

    auto unwrapped_arg = (gmal_thread_routine_arg*)arg;

    // Simulates being blocked at thread creation -> THREAD_START for this thread
    thread_await_gmal_scheduler_for_thread_start_transition();
    void * return_value = unwrapped_arg->routine(unwrapped_arg->arg);

    // Simulates being blocked after the thread exits
//    dpor_post_thread_operation_to_parent(tid_self, THREAD_FINISH);
    thread_await_gmal_scheduler();

    // See where the thread_wrapper is created. The memory is malloc'ed and should be freed
    free(arg);

//    dpor_post_thread_operation_to_parent(tid_self, THREAD_FINISH);
    thread_awake_gmal_scheduler_for_thread_finish_transition();
    return return_value;
}

int
gmal_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*routine)(void *), void *arg)
{
    GMAL_FATAL_ON_FAIL(attr == nullptr); // TODO: For now, we don't support attributes. This should be added in the future

    auto dpor_thread_arg = (gmal_thread_routine_arg *)malloc(sizeof(gmal_thread_routine_arg));
    dpor_thread_arg->arg = arg;
    dpor_thread_arg->routine = routine;

    int return_value = pthread_create(thread, attr, &gmal_thread_routine_wrapper, dpor_thread_arg);

    // We need to ensure that the thread is
    // created has fully registered itself with the
    // concurrent system; otherwise, there is a race condition
    // in which two thread creates in the child might
    // not be scheduled to run until *two* steps of the scheduler
    sem_wait(&gmal_pthread_create_binary_sem);

    // pthread_create may fail to actually spawn a thread
    tid_t tid_return = return_value == 0 ? TID_INVALID : TID_PTHREAD_CREATE_FAILED;
//    dpor_post_thread_operation_to_parent_with_target(tid_return, THREAD_CREATE, *thread);
    thread_await_gmal_scheduler();

    return return_value;
}

// NOTE: Assumes that the parent process
// is asleep (called dpor_run_thread_to_next_visible_operation); the behavior
// is undefined otherwise
void
thread_await_gmal_scheduler()
{
    GMAL_ASSERT(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid_self];
    mc_shared_cv_wake_scheduler(cv);
    mc_shared_cv_wait_for_scheduler(cv);
}

// NOTE: This should only be called in one location:
// When the scheduler starts, there is an initial
// race condition between the child process and the
// parent process with `thread_await_dpor_scheduler`. `thread_await_dpor_scheduler` assumes
// the scheduler (parent) process is asleep; but upon
// initialization this is not true. Hence, this method is invoked instead
void
thread_await_gmal_scheduler_for_thread_start_transition()
{
    GMAL_ASSERT(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid_self];
    mc_shared_cv_wait_for_scheduler(cv);
}

void
thread_awake_gmal_scheduler_for_thread_finish_transition() {
    GMAL_ASSERT(tid_self != TID_INVALID);
    mc_shared_cv_ref cv = &(*threadQueue)[tid_self];
    mc_shared_cv_wake_scheduler(cv);
}