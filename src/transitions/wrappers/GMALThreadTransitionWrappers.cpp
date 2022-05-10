#include "GMALThreadTransitionWrappers.h"
#include "GMAL.h"
#include "objects/GMALThread.h"
#include "transitions/GMALThreadCreate.h"
#include "transitions/GMALThreadFinish.h"
#include "transitions/GMALThreadJoin.h"
#include "transitions/GMALTransitionsShared.h"
#include "transitions/GMALExitTransition.h"
#include <typeinfo>

extern "C" {
    #include "GMALSharedLibraryWrappers.h"
}

struct gmal_thread_routine_arg {
    void *arg;
    thread_routine routine;
};

void *
gmal_thread_routine_wrapper(void * arg)
{
    tid_self = programState->createNewThread();
    __real_sem_post(&gmal_pthread_create_binary_sem);

    auto unwrapped_arg = (gmal_thread_routine_arg*)arg;

    // Simulates being blocked at thread creation -> THREAD_START for this thread
    // NOTE: Don't write into shared memory here! The scheduler already knows how to
    // handle the case of thread creation
    thread_await_gmal_scheduler_for_thread_start_transition();
    void * return_value = unwrapped_arg->routine(unwrapped_arg->arg);

    // Simulates being blocked after the thread exits
    // NOTE: Thread exit requires only data about the thread that ran
    thread_post_visible_operation_hit(typeid(GMALThreadFinish));
    thread_await_gmal_scheduler();

    // See where the thread_wrapper is created. The memory is malloc'ed and should be freed
    free(arg);

    // NOTE: Thread exit requires only data about the thread that ran
    thread_post_visible_operation_hit(typeid(GMALThreadFinish));
    thread_awake_gmal_scheduler_for_thread_finish_transition();
    return return_value;
}

int
gmal_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*routine)(void *), void *arg)
{
    // TODO: For now, we don't support attributes. This should be added in the future
    if (attr != nullptr) {
        gmal_child_panic();
    }

    auto dpor_thread_arg = (gmal_thread_routine_arg *)malloc(sizeof(gmal_thread_routine_arg));
    dpor_thread_arg->arg = arg;
    dpor_thread_arg->routine = routine;

    int return_value = __real_pthread_create(thread, attr, &gmal_thread_routine_wrapper, dpor_thread_arg);

    // We need to ensure that the thread is
    // created has fully registered itself with the
    // concurrent system; otherwise, there is a race condition
    // in which two thread creates in the child might
    // not be scheduled to run until *two* steps of the scheduler
    __real_sem_wait(&gmal_pthread_create_binary_sem);
    // TODO: When pthread_create fails, *thread is undefined
    auto newlyCreatedThread = GMALThreadShadow(arg, routine, *thread);
    thread_post_visible_operation_hit<GMALThreadShadow>(typeid(GMALThreadCreate), &newlyCreatedThread);
    thread_await_gmal_scheduler();

    return return_value;
}

int
gmal_pthread_join(pthread_t thread, void **output)
{
    // The join handler doesn't care about the other arguments
    auto newlyCreatedThread = GMALThreadShadow(nullptr, nullptr, thread);
    thread_post_visible_operation_hit<GMALThreadShadow>(typeid(GMALThreadJoin), &newlyCreatedThread);
    thread_await_gmal_scheduler();

    // TODO: What should we do when this fails
    return __real_pthread_join(thread, output);
}

void
gmal_exit_main_thread()
{
    auto newlyCreatedThread = GMALThreadShadow(nullptr, nullptr, pthread_self());
    thread_post_visible_operation_hit(typeid(GMALThreadFinish), &newlyCreatedThread);
    thread_await_gmal_scheduler();
}

GMAL_NO_RETURN void
gmal_exit(int status)
{
    thread_post_visible_operation_hit(typeid(GMALExitTransition), &status);
    thread_await_gmal_scheduler();
    __real_exit(status);
}