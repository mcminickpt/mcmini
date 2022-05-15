#include "GMALConditionVariableWrappers.h"

#include "transitions/GMALTransitionsShared.h"
#include "transitions/cond/GMALCondInit.h"
#include "transitions/cond/GMALCondWait.h"
#include "transitions/cond/GMALCondBroadcast.h"
#include "transitions/cond/GMALCondSignal.h"
#include "transitions/cond/GMALCondEnqueue.h"

extern "C" {
    #include "transitions/wrappers/GMALSharedLibraryWrappers.h"
}

int
gmal_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    thread_post_visible_operation_hit(typeid(GMALCondInit), &cond);
    thread_await_gmal_scheduler();
    return __real_pthread_cond_init(cond, attr);
}

int
gmal_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{

    const auto condPlusMutex = GMALSharedMemoryConditionVariable(cond, mutex);
    thread_post_visible_operation_hit(typeid(GMALCondEnqueue), &condPlusMutex);
    thread_await_gmal_scheduler();
    __real_pthread_mutex_unlock(mutex);
    thread_post_visible_operation_hit(typeid(GMALCondWait), &condPlusMutex);
    thread_await_gmal_scheduler();
    __real_pthread_mutex_lock(mutex);
    return 0;
}

int
gmal_pthread_cond_signal(pthread_cond_t *cond)
{
    thread_post_visible_operation_hit(typeid(GMALCondSignal), &cond);
    thread_await_gmal_scheduler();
    return 0;
}

int
gmal_pthread_cond_broadcast(pthread_cond_t *cond)
{
    thread_post_visible_operation_hit(typeid(GMALCondBroadcast), &cond);
    thread_await_gmal_scheduler();
    return 0;
}