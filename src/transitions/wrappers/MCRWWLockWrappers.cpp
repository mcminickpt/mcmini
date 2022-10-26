#include "mcmini/objects/MCRWWLock.h"
#include "mcmini/transitions/MCTransitionsShared.h"
#include "mcmini/transitions/wrappers/MCRWLockWrappers.h"

extern "C" {
#include "mcmini/transitions/wrappers/MCSharedLibraryWrappers.h"
}

int
mc_pthread_rwwlock_init(pthread_rwwlock_t *rwwlock)
{
  MCRWWLockShadow lock(rwwlock);
  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockInit), &lock);
  thread_await_scheduler();

  /* Here we can show the power of mcmini -- the true method need not
   * even exist! */
  return 0;
}

int
mc_pthread_rwwlock_rdlock(pthread_rwwlock_t *rwwlock)
{
  MCRWWLockShadow lock(rwwlock);
  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockReaderEnqueue), &lock);
  thread_await_scheduler();

  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockReaderLock), &lock);
  thread_await_scheduler();

  /* Here we can show the power of mcmini -- the true method need not
   * even exist! */
  return 0;
}

int
mc_pthread_rwwlock_wr1lock(pthread_rwwlock_t *rwwlock)
{
  MCRWWLockShadow lock(rwwlock);
  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockWriter1Enqueue), &lock);
  thread_await_scheduler();

  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockWriter1Lock), &lock);
  thread_await_scheduler();

  /* Here we can show the power of mcmini -- the true method need not
   * even exist! */
  return 0;
}

int
mc_pthread_rwwlock_wr2lock(pthread_rwwlock_t *rwwlock)
{
  MCRWWLockShadow lock(rwwlock);
  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockWriter2Enqueue), &lock);
  thread_await_scheduler();

  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockWriter2Lock), &lock);
  thread_await_scheduler();

  /* Here we can show the power of mcmini -- the true method need not
   * even exist! */
  return 0;
}

int
mc_pthread_rwwlock_unlock(pthread_rwwlock_t *rwwlock)
{
  MCRWWLockShadow lock(rwwlock);
  thread_post_visible_operation_hit<MCRWWLockShadow>(
    typeid(MCRWWLockUnlock), &lock);
  thread_await_scheduler();

  /* Here we can show the power of mcmini -- the true method need not
   * even exist! */
  return 0;
}

int
mc_pthread_rwwlock_destroy(pthread_rwwlock_t *rwwlock)
{
  /* Unimplemented at the moment */
  return 0;
}