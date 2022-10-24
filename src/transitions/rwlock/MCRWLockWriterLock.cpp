#include "mcmini/transitions/rwlock/MCRWLockWriterLock.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/rwlock/MCRWLockReaderLock.h"

MCTransition *
MCReadRWLockWriterLock(const MCSharedTransition *shmTransition,
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
  return new MCRWLockWriterLock(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockWriterLock::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockWriterLock>(threadCpy, rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockWriterLock::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockWriterLock>(threadInState,
                                              liveRWLock);
}

void
MCRWLockWriterLock::applyToState(MCState *state)
{
  this->rwlock->writer_lock(this->getThreadId());
}

bool
MCRWLockWriterLock::enabledInState(const MCState *state) const
{
  return this->rwlock->canAcquireAsWriter(this->getThreadId());
}

bool
MCRWLockWriterLock::coenabledWith(const MCTransition *other) const
{
  {
    const MCRWLockReaderLock *maybeReaderLock =
      dynamic_cast<const MCRWLockReaderLock *>(other);
    if (maybeReaderLock) {
      return *maybeReaderLock->rwlock != *this->rwlock;
    }
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
MCRWLockWriterLock::dependentWith(const MCTransition *other) const
{
  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockWriterLock::print() const
{
  printf("thread %lu: pthread_rwlock_wrlock(%lu) (lock)\n",
         this->thread->tid, this->rwlock->getObjectId());
}