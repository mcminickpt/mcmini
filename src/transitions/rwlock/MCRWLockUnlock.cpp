#include "transitions/rwlock/MCRWLockUnlock.h"
#include "mcmini_private.h"

MCTransition *
MCReadRWLockUnlock(const MCSharedTransition *shmTransition,
                   void *shmData, MCStack *state)
{
  auto rwlockInShm = static_cast<MCRWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWLock>(systemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr,
    "Attempting to unlock an uninitialized rw-lock");

  if (rwLock->isUnlocked()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a rw-lock that is already unlocked");
  } else if (rwLock->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a rw-lock that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWLockUnlock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockUnlock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockUnlock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockUnlock::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockUnlock>(threadInState, liveRWLock);
}

void
MCRWLockUnlock::applyToState(MCStack *state)
{
  this->rwlock->unlock(this->getThreadId());
}

bool
MCRWLockUnlock::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWLockUnlock::dependentWith(const MCTransition *other) const
{
  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockUnlock::print() const
{
  mcprintf("thread %lu: pthread_rwlock_unlock(rwl:%u)\n", this->thread->tid,
           countVisibleObjectsOfType(this->rwlock->getObjectId()));
}
