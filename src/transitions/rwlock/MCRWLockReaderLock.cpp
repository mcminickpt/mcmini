#include "mcmini/transitions/rwlock/MCRWLockReaderLock.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/rwlock/MCRWLockWriterLock.h"

MCTransition *
MCReadRWLockReaderLock(const MCSharedTransition *shmTransition,
                       void *shmData, MCState *state)
{
  auto rwlockInShm = static_cast<MCRWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWLock>(systemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr, "Attempting to lock an uninitialized rw-lock");

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !rwLock->isDestroyed(), "Attempting to lock a destroyed rw-lock");

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWLockReaderLock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockReaderLock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockReaderLock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockReaderLock::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockReaderLock>(threadInState,
                                              liveRWLock);
}

void
MCRWLockReaderLock::applyToState(MCState *state)
{
  this->rwlock->reader_lock(this->getThreadId());
}

bool
MCRWLockReaderLock::enabledInState(const MCState *state) const
{
  return this->rwlock->canAcquireAsReader(this->getThreadId());
}

bool
MCRWLockReaderLock::coenabledWith(const MCTransition *other) const
{
  {
    const MCRWLockReaderLock *maybeReaderLock =
      dynamic_cast<const MCRWLockReaderLock *>(other);
    if (maybeReaderLock) { return true; }
  }

  {
    const MCRWLockWriterLock *maybeWriterLock =
      dynamic_cast<const MCRWLockWriterLock *>(other);
    if (maybeWriterLock) {
      return *maybeWriterLock->rwlock != *this->rwlock;
    }
  }

  return true;
}

bool
MCRWLockReaderLock::dependentWith(const MCTransition *other) const
{
  // Independent with all other reader lock operations
  const MCRWLockReaderLock *maybeRWLockReaderOperation =
    dynamic_cast<const MCRWLockReaderLock *>(other);
  if (maybeRWLockReaderOperation) { return false; }

  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockReaderLock::print() const
{
  printf("thread %lu: pthread_rwlock_rdlock(%lu) (lock)\n",
         this->thread->tid, this->rwlock->getObjectId());
}