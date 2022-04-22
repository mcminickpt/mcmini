#include "GMALMutexTransitionWrappers.h"
#include "GMALTransitionsShared.h"
#include "GMALMutexLock.h"
#include "GMALMutexUnlock.h"
#include "GMALMutexInit.h"
#include "GMAL.h"

extern "C" {
    #include "GMALSharedLibraryWrappers.h"
}

template<typename SharedMemoryData> void
thread_post_visible_operation_hit(const std::type_info &type, SharedMemoryData * shmData)
{
    auto newTypeInfo = GMALSharedTransition(tid_self, type);
    auto newShmData = shmData;
    memcpy(shmTransitionTypeInfo, &newTypeInfo, sizeof(GMALSharedTransition));
    memcpy(shmTransitionData, newShmData, sizeof(SharedMemoryData));
}

int
gmal_pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    // The join handler doesn't care about the other arguments
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
