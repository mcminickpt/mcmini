#include "MCThreadTransitionWrappers.h"
#include "MCMINI.h"
#include "objects/MCThread.h"
#include "transitions/threads/MCThreadCreate.h"
#include "transitions/threads/MCThreadFinish.h"
#include "transitions/threads/MCThreadJoin.h"
#include "transitions/MCTransitionsShared.h"
#include "transitions/misc/MCExitTransition.h"
#include <typeinfo>

extern "C" {
    #include "MCSharedLibraryWrappers.h"
}

struct mc_thread_routine_arg {
    void *arg;
    thread_routine routine;
};

void *
mc_thread_routine_wrapper(void * arg)
{
    tid_self = programState->createNewThread();
    __real_sem_post(&mc_pthread_create_binary_sem);

    auto unwrapped_arg = (mc_thread_routine_arg*)arg;

    // Simulates being blocked at thread creation -> THREAD_START for this thread
    // NOTE: Don't write into shared memory here! The scheduler already knows how to
    // handle the case of thread creation
    thread_await_mc_scheduler_for_thread_start_transition();
    void * return_value = unwrapped_arg->routine(unwrapped_arg->arg);

    // Simulates being blocked after the thread exits
    // NOTE: Thread exit requires only data about the thread that ran
    thread_post_visible_operation_hit(typeid(MCThreadFinish));
    thread_await_mc_scheduler();

    // See where the thread_wrapper is created. The memory is malloc'ed and should be freed
    free(arg);

    // NOTE: Thread exit requires only data about the thread that ran
    thread_post_visible_operation_hit(typeid(MCThreadFinish));
    thread_awake_mc_scheduler_for_thread_finish_transition();
    return return_value;
}

int
mc_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*routine)(void *), void *arg)
{
    // TODO: For now, we don't support attributes. This should be added in the future
    if (attr != nullptr) {
        mc_child_panic();
    }

    auto dpor_thread_arg = (mc_thread_routine_arg *)malloc(sizeof(mc_thread_routine_arg));
    dpor_thread_arg->arg = arg;
    dpor_thread_arg->routine = routine;

    int return_value = __real_pthread_create(thread, attr, &mc_thread_routine_wrapper, dpor_thread_arg);

    // We need to ensure that the thread is
    // created has fully registered itself with the
    // concurrent system; otherwise, there is a race condition
    // in which two thread creates in the child might
    // not be scheduled to run until *two* steps of the scheduler
    __real_sem_wait(&mc_pthread_create_binary_sem);
    // TODO: When pthread_create fails, *thread is undefined
    auto newlyCreatedThread = MCThreadShadow(arg, routine, *thread);
    thread_post_visible_operation_hit<MCThreadShadow>(typeid(MCThreadCreate), &newlyCreatedThread);
    thread_await_mc_scheduler();

    return return_value;
}

int
mc_pthread_join(pthread_t thread, void **output)
{
    // The join handler doesn't care about the other arguments
    auto newlyCreatedThread = MCThreadShadow(nullptr, nullptr, thread);
    thread_post_visible_operation_hit<MCThreadShadow>(typeid(MCThreadJoin), &newlyCreatedThread);
    thread_await_mc_scheduler();

    // TODO: What should we do when this fails
    return __real_pthread_join(thread, output);
}

void
mc_exit_main_thread()
{
    auto newlyCreatedThread = MCThreadShadow(nullptr, nullptr, pthread_self());
    thread_post_visible_operation_hit(typeid(MCThreadFinish), &newlyCreatedThread);
    thread_await_mc_scheduler();
}

MC_NO_RETURN void
mc_exit(int status)
{
    thread_post_visible_operation_hit(typeid(MCExitTransition), &status);
    thread_await_mc_scheduler();
    __real_exit(status);
}

template<typename T> void
mc_pthread_reach_point()
{
    tid_t thread = tid_self;
    thread_post_visible_operation_hit(typeid(T), &thread);
    thread_await_mc_scheduler();
}

void
mc_pthread_reach_goal()
{
    mc_pthread_reach_point<MCThreadReachGoal>();
}

void
mc_pthread_enter_goal_critical_section()
{
    mc_pthread_reach_point<MCThreadEnterGoalCriticalSection>();
}

void
mc_pthread_exit_goal_critical_section()
{
    mc_pthread_reach_point<MCThreadExitGoalCriticalSection>();
}

