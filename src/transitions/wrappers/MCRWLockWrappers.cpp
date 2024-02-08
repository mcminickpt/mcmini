#include "mcmini/transitions/wrappers/MCRWLockWrappers.h"
#include "mcmini/objects/MCRWLock.h"
#include "mcmini/transitions/MCTransitionsShared.h"

extern "C" {
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"
}

int
mc_pthread_rwlock_init(pthread_rwlock_t *rwlock,
                       const pthread_rwlockattr_t *attr)
{
  MCRWLockShadow newLock(rwlock);
  thread_post_visible_operation_hit<MCRWLockShadow>(
    typeid(MCRWLockInit), &newLock);
  thread_await_scheduler();

  return __real_pthread_rwlock_init(rwlock, attr);
}

int
mc_pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
  MCRWLockShadow lock(rwlock);

  thread_post_visible_operation_hit<MCRWLockShadow>(
    typeid(MCRWLockReaderEnqueue), &lock);
  thread_await_scheduler();

  thread_post_visible_operation_hit<MCRWLockShadow>(
    typeid(MCRWLockReaderLock), &lock);
  thread_await_scheduler();

  return __real_pthread_rwlock_rdlock(rwlock);
}

int
mc_pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
  MCRWLockShadow lock(rwlock);

  thread_post_visible_operation_hit<MCRWLockShadow>(
    typeid(MCRWLockWriterEnqueue), &lock);
  thread_await_scheduler();

  thread_post_visible_operation_hit<MCRWLockShadow>(
    typeid(MCRWLockWriterLock), &lock);
  thread_await_scheduler();

  return __real_pthread_rwlock_wrlock(rwlock);
}

int
mc_pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
  MCRWLockShadow lock(rwlock);
  thread_post_visible_operation_hit<MCRWLockShadow>(
    typeid(MCRWLockUnlock), &lock);
  thread_await_scheduler();

  return __real_pthread_rwlock_unlock(rwlock);
}

int
mc_pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
  // Unimplemented at the moment...
  return 0;
}