#include "mcmini/transitions/rwwlock/MCRWWLockWriter2Lock.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/rwwlock/MCRWWLockReaderLock.h"
#include "mcmini/transitions/rwwlock/MCRWWLockWriter2Lock.h"

MCTransition *
MCReadRWWLockWriter2Lock(const MCSharedTransition *shmTransition,
                         void *shmData, MCState *state)
{
  auto rwlockInShm = static_cast<MCRWWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWWLock>(systemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr, "Attempting to lock an uninitialized rw-lock");

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !rwLock->isDestroyed(), "Attempting to lock a destroyed rw-lock");

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWWLockWriter2Lock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWWLockWriter2Lock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWWLock, MCVisibleObject>(
      this->rwwlock->copy());
  return std::make_shared<MCRWWLockWriter2Lock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWWLockWriter2Lock::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWWLock> liveRWLock =
    state->getObjectWithId<MCRWWLock>(rwwlock->getObjectId());
  return std::make_shared<MCRWWLockWriter2Lock>(threadInState,
                                                liveRWLock);
}

void
MCRWWLockWriter2Lock::applyToState(MCState *state)
{
  this->rwwlock->writer2_lock(this->getThreadId());
}

bool
MCRWWLockWriter2Lock::enabledInState(const MCState *state) const
{
  return this->rwwlock->canAcquireAsWriter2(this->getThreadId());
}

bool
MCRWWLockWriter2Lock::coenabledWith(const MCTransition *other) const
{
  {
    const MCRWWLockReaderLock *maybeReaderLock =
      dynamic_cast<const MCRWWLockReaderLock *>(other);
    if (maybeReaderLock) {
      return *maybeReaderLock->rwwlock != *this->rwwlock;
    }
  }

  {
    const MCRWWLockWriter2Lock *maybeWriterLock =
      dynamic_cast<const MCRWWLockWriter2Lock *>(other);
    if (maybeWriterLock) {
      return *maybeWriterLock->rwwlock != *this->rwwlock;
    }
  }

  {
    const MCRWWLockWriter2Lock *maybeWriterLock =
      dynamic_cast<const MCRWWLockWriter2Lock *>(other);
    if (maybeWriterLock) {
      return *maybeWriterLock->rwwlock != *this->rwwlock;
    }
  }
  return true;
}

bool
MCRWWLockWriter2Lock::dependentWith(const MCTransition *other) const
{
  const MCRWWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwwlock == *this->rwwlock;
  }
  return false;
}

void
MCRWWLockWriter2Lock::print() const
{
  printf("thread %lu: pthread_rwlock_wr2lock(%lu) (lock)\n",
         this->thread->tid, this->rwwlock->getObjectId());
}