#include "mcmini/transitions/wrappers/MCMutexTransitionWrappers.h"
#include "mcmini/transitions/MCTransitionsShared.h"
#include "mcmini/transitions/mutex/MCMutexInit.h"
#include "mcmini/transitions/mutex/MCMutexLock.h"
#include "mcmini/transitions/mutex/MCMutexUnlock.h"

extern "C" {
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"
}

int
mc_pthread_mutex_init(pthread_mutex_t *mutex,
                      const pthread_mutexattr_t *attr)
{
  // The handler doesn't care about the other arguments
  auto newlyCreatedMutex = MCMutexShadow(mutex);
  thread_post_visible_operation_hit<MCMutexShadow>(
    typeid(MCMutexInit), &newlyCreatedMutex);
  thread_await_scheduler();

  // TODO: What should we do when this fails
  return __real_pthread_mutex_init(mutex, attr);
}

int
mc_pthread_mutex_lock(pthread_mutex_t *mutex)
{
  // The join handler doesn't care about the other arguments
  auto newlyCreatedMutex = MCMutexShadow(mutex);
  thread_post_visible_operation_hit<MCMutexShadow>(
    typeid(MCMutexLock), &newlyCreatedMutex);
  thread_await_scheduler();

  // TODO: What should we do when this fails
  return __real_pthread_mutex_lock(mutex);
}

int
mc_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  // The join handler doesn't care about the other arguments
  auto newlyCreatedMutex = MCMutexShadow(mutex);
  thread_post_visible_operation_hit<MCMutexShadow>(
    typeid(MCMutexUnlock), &newlyCreatedMutex);
  thread_await_scheduler();

  // TODO: What should we do when this fails
  return __real_pthread_mutex_unlock(mutex);
}
