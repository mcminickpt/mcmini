#include "MCConditionVariableWrappers.h"

#include "transitions/MCTransitionsShared.h"
#include "transitions/MCCondInit.h"
#include "transitions/MCCondWait.h"
#include "transitions/MCCondBroadcast.h"
#include "transitions/MCCondSignal.h"
#include "transitions/MCCondEnqueue.h"

extern "C" {
    #include "transitions/wrappers/MCSharedLibraryWrappers.h"
}

int
mc_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    thread_post_visible_operation_hit(typeid(MCCondInit), &cond);
    thread_await_mc_scheduler();
    return __real_pthread_cond_init(cond, attr);
}

int
mc_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{

    const auto condPlusMutex = MCSharedMemoryConditionVariable(cond, mutex);
    thread_post_visible_operation_hit(typeid(MCCondEnqueue), &condPlusMutex);
    thread_await_mc_scheduler();
    __real_pthread_mutex_unlock(mutex);
    thread_post_visible_operation_hit(typeid(MCCondWait), &condPlusMutex);
    thread_await_mc_scheduler();
    __real_pthread_mutex_lock(mutex);
    return 0;
}

int
mc_pthread_cond_signal(pthread_cond_t *cond)
{
    thread_post_visible_operation_hit(typeid(MCCondSignal), &cond);
    thread_await_mc_scheduler();
    return 0;
}

int
mc_pthread_cond_broadcast(pthread_cond_t *cond)
{
    thread_post_visible_operation_hit(typeid(MCCondBroadcast), &cond);
    thread_await_mc_scheduler();
    return 0;
}