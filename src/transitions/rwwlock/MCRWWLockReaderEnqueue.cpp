#include "mcmini/transitions/rwwlock/MCRWWLockReaderEnqueue.h"
#include "mcmini/mcmini_private.h"

using namespace std;

MCTransition *
MCReadRWWLockReaderEnqueue(const MCSharedTransition *shmTransition,
                           void *shmData, MCState *state)
{
  auto rwlockInShm = static_cast<MCRWWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWWLock>(systemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr, "Attempting to lock an uninitialized rw-lock");

  if (rwLock->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a rwwlock that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWWLockReaderEnqueue(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWWLockReaderEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWWLock, MCVisibleObject>(
      this->rwwlock->copy());
  return std::make_shared<MCRWWLockReaderEnqueue>(threadCpy,
                                                  rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWWLockReaderEnqueue::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWWLock> liveRWLock =
    state->getObjectWithId<MCRWWLock>(rwwlock->getObjectId());
  return std::make_shared<MCRWWLockReaderEnqueue>(threadInState,
                                                  liveRWLock);
}

void
MCRWWLockReaderEnqueue::applyToState(MCState *state)
{
  // Enqueue this thread as a reader
  this->rwwlock->enqueue_as_reader(this->getThreadId());
}

bool
MCRWWLockReaderEnqueue::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWWLockReaderEnqueue::dependentWith(const MCTransition *other) const
{
  const MCRWWLockTransition *maybeRWWLockOperation =
    dynamic_cast<const MCRWWLockTransition *>(other);
  if (maybeRWWLockOperation) {
    return *maybeRWWLockOperation->rwwlock == *this->rwwlock;
  }
  return false;
}

void
MCRWWLockReaderEnqueue::print() const
{
  printf("thread %lu: pthread_rwwlock_rdlock(%lu) (wait)\n",
         this->thread->tid, this->rwwlock->getObjectId());
}