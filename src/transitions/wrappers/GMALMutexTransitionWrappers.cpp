#include "GMALMutexTransitionWrappers.h"
#include "transitions/GMALTransitionsShared.h"
#include "transitions/mutex/GMALMutexLock.h"
#include "transitions/mutex/GMALMutexUnlock.h"
#include "transitions/mutex/GMALMutexInit.h"

extern "C" {
    #include "transitions/wrappers/GMALSharedLibraryWrappers.h"
}

int
gmal_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    // The handler doesn't care about the other arguments
    auto newlyCreatedMutex = GMALMutexShadow(mutex);
    thread_post_visible_operation_hit<GMALMutexShadow>(typeid(GMALMutexInit), &newlyCreatedMutex);
    thread_await_gmal_scheduler();

    // TODO: What should we do when this fails
    return __real_pthread_mutex_init(mutex, attr);
}

int
gmal_pthread_mutex_lock(pthread_mutex_t *mutex)
{
    // The join handler doesn't care about the other arguments
    auto newlyCreatedMutex = GMALMutexShadow(mutex);
    thread_post_visible_operation_hit<GMALMutexShadow>(typeid(GMALMutexLock), &newlyCreatedMutex);
    thread_await_gmal_scheduler();

    // TODO: What should we do when this fails
    return __real_pthread_mutex_lock(mutex);
}

int
gmal_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    // The join handler doesn't care about the other arguments
    auto newlyCreatedMutex = GMALMutexShadow(mutex);
    thread_post_visible_operation_hit<GMALMutexShadow>(typeid(GMALMutexUnlock), &newlyCreatedMutex);
    thread_await_gmal_scheduler();

    // TODO: What should we do when this fails
    return __real_pthread_mutex_unlock(mutex);
}
