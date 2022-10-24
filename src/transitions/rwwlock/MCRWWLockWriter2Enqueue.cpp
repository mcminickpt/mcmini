#include "mcmini/transitions/rwwlock/MCRWWLockWriter2Enqueue.h"
#include "mcmini/mcmini_private.h"

using namespace std;

MCTransition *
MCReadRWWLockWriter2Enqueue(const MCSharedTransition *shmTransition,
                            void *shmData, MCState *state)
{
  auto rwlockInShm = static_cast<MCRWWLockShadow *>(shmData);
  auto systemId    = (MCSystemID)rwlockInShm->systemIdentity;
  auto rwLock =
    state->getVisibleObjectWithSystemIdentity<MCRWWLock>(systemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    rwLock != nullptr, "Attempting to an uninitialized rw-lock");

  if (rwLock->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to unlock a rww-lock that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCRWWLockWriter2Enqueue(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWWLockWriter2Enqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWWLock, MCVisibleObject>(
      this->rwwlock->copy());
  return std::make_shared<MCRWWLockWriter2Enqueue>(threadCpy,
                                                   rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWWLockWriter2Enqueue::dynamicCopyInState(
  const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWWLock> liveRWLock =
    state->getObjectWithId<MCRWWLock>(rwwlock->getObjectId());
  return std::make_shared<MCRWWLockWriter2Enqueue>(threadInState,
                                                   liveRWLock);
}

void
MCRWWLockWriter2Enqueue::applyToState(MCState *state)
{
  // Enqueue this thread as a writer 1 type
  this->rwwlock->enqueue_as_writer2(this->getThreadId());
}

bool
MCRWWLockWriter2Enqueue::coenabledWith(
  const MCTransition *other) const
{
  return true;
}

bool
MCRWWLockWriter2Enqueue::dependentWith(
  const MCTransition *other) const
{
  const MCRWWLockTransition *maybeRWWLockOperation =
    dynamic_cast<const MCRWWLockTransition *>(other);
  if (maybeRWWLockOperation) {
    return *maybeRWWLockOperation->rwwlock == *this->rwwlock;
  }
  return false;
}

void
MCRWWLockWriter2Enqueue::print() const
{
  printf("thread %lu: pthread_rwwlock_wr2lock(%lu) (wait)\n",
         this->thread->tid, this->rwwlock->getObjectId());
}