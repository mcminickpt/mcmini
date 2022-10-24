#include "mcmini/transitions/rwwlock/MCRWWLockUnlock.h"
#include "mcmini/mcmini_private.h"

MCTransition *
MCReadRWWLockUnlock(const MCSharedTransition *shmTransition,
                    void *shmData, MCState *state)
{
  auto rwlockInShm = static_cast<MCRWWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWWLock>(systemId);

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
  return new MCRWWLockUnlock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWWLockUnlock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWWLock, MCVisibleObject>(
      this->rwwlock->copy());
  return std::make_shared<MCRWWLockUnlock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWWLockUnlock::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWWLock> liveRWLock =
    state->getObjectWithId<MCRWWLock>(rwwlock->getObjectId());
  return std::make_shared<MCRWWLockUnlock>(threadInState, liveRWLock);
}

void
MCRWWLockUnlock::applyToState(MCState *state)
{
  this->rwwlock->unlock(this->getThreadId());
}

bool
MCRWWLockUnlock::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWWLockUnlock::dependentWith(const MCTransition *other) const
{
  const MCRWWLockTransition *maybeRWWLockOperation =
    dynamic_cast<const MCRWWLockTransition *>(other);
  if (maybeRWWLockOperation) {
    return *maybeRWWLockOperation->rwwlock == *this->rwwlock;
  }
  return false;
}

void
MCRWWLockUnlock::print() const
{
  printf("thread %lu: pthread_rwwlock_unlock(%lu)\n",
         this->thread->tid, this->rwwlock->getObjectId());
}