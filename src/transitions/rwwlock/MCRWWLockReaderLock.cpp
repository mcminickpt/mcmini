#include "transitions/rwwlock/MCRWWLockReaderLock.h"
#include "mcmini_private.h"
#include "transitions/rwwlock/MCRWWLockWriter1Lock.h"
#include "transitions/rwwlock/MCRWWLockWriter2Lock.h"

MCTransition *
MCReadRWWLockReaderLock(const MCSharedTransition *shmTransition,
                        void *shmData, MCStack *state)
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
  return new MCRWWLockReaderLock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWWLockReaderLock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWWLock, MCVisibleObject>(
      this->rwwlock->copy());
  return std::make_shared<MCRWWLockReaderLock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWWLockReaderLock::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWWLock> liveRWLock =
    state->getObjectWithId<MCRWWLock>(rwwlock->getObjectId());
  return std::make_shared<MCRWWLockReaderLock>(threadInState,
                                               liveRWLock);
}

void
MCRWWLockReaderLock::applyToState(MCStack *state)
{
  this->rwwlock->reader_lock(this->getThreadId());
}

bool
MCRWWLockReaderLock::enabledInState(const MCStack *state) const
{
  return this->rwwlock->canAcquireAsReader(this->getThreadId());
}

bool
MCRWWLockReaderLock::coenabledWith(const MCTransition *other) const
{
  {
    const MCRWWLockReaderLock *maybeReaderLock =
      dynamic_cast<const MCRWWLockReaderLock *>(other);
    if (maybeReaderLock) { return true; }
  }

  {
    const MCRWWLockWriter1Lock *maybeWriterLock =
      dynamic_cast<const MCRWWLockWriter1Lock *>(other);
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
MCRWWLockReaderLock::dependentWith(const MCTransition *other) const
{
  // Independent with all other reader lock operations
  const MCRWWLockReaderLock *maybeRWLockReaderOperation =
    dynamic_cast<const MCRWWLockReaderLock *>(other);
  if (maybeRWLockReaderOperation) { return false; }

  const MCRWWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwwlock == *this->rwwlock;
  }

  return false;
}

MCTransitionUniqueRep
MCRWWLockReaderLock::toUniqueRep() const
{
  MCTransitionUniqueRep uniqueRep;
  uniqueRep.typeId = MC_RWW_LOCK_READER_LOCK;
  uniqueRep.threadId = this->thread->tid;
  uniqueRep.param.val[0] = countVisibleObjectsOfType(this->rwwlock->
                                                        getObjectId()); 
  uniqueRep.param.val[1] = UINT_MAX;
  return uniqueRep;
}

void
MCRWWLockReaderLock::print() const
{
  mcprintf("thread %lu: pthread_rwwlock_rdlock(rwwl:%u) (lock)\n",
           this->thread->tid,
           countVisibleObjectsOfType(this->rwwlock->getObjectId()));
}
