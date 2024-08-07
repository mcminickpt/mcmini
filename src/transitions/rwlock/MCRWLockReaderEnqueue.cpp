#include "transitions/rwlock/MCRWLockReaderEnqueue.h"
#include "mcmini_private.h"

using namespace std;

MCTransition *
MCReadRWLockReaderEnqueue(const MCSharedTransition *shmTransition,
                          void *shmData, MCStack *state)
{
  auto rwlockInShm = static_cast<MCRWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWLock>(systemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr, "Attempting to  an uninitialized rw-lock");

  if (rwLock->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a rw-lock that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWLockReaderEnqueue(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockReaderEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockReaderEnqueue>(threadCpy,
                                                 rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockReaderEnqueue::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockReaderEnqueue>(threadInState,
                                                 liveRWLock);
}

void
MCRWLockReaderEnqueue::applyToState(MCStack *state)
{
  // Enqueue this thread as a reader
  this->rwlock->enqueue_as_reader(this->getThreadId());
}

bool
MCRWLockReaderEnqueue::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWLockReaderEnqueue::dependentWith(const MCTransition *other) const
{
  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockReaderEnqueue::print() const
{
  mcprintf("thread %lu: pthread_rwlock_rdlock(rwl:%u) (wait)\n",
           this->thread->tid,
           countVisibleObjectsOfType(this->rwlock->getObjectId()));
}
