#include "mcmini/transitions/rwlock/MCRWLockWriterEnqueue.h"
#include "mcmini/mcmini_private.h"

using namespace std;

MCTransition *
MCReadRWLockWriterEnqueue(const MCSharedTransition *shmTransition,
                          void *shmData, MCState *state)
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
  return new MCRWLockWriterEnqueue(threadThatRan, rwLock);
}

std::shared_ptr<MCTransition>
MCRWLockWriterEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto rwlockCpy =
    std::static_pointer_cast<MCRWLock, MCVisibleObject>(
      this->rwlock->copy());
  return std::make_shared<MCRWLockWriterEnqueue>(threadCpy,
                                                 rwlockCpy);
}

std::shared_ptr<MCTransition>
MCRWLockWriterEnqueue::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCRWLock> liveRWLock =
    state->getObjectWithId<MCRWLock>(rwlock->getObjectId());
  return std::make_shared<MCRWLockWriterEnqueue>(threadInState,
                                                 liveRWLock);
}

void
MCRWLockWriterEnqueue::applyToState(MCState *state)
{
  // Enqueue this thread as a writer
  this->rwlock->enqueue_as_writer(this->getThreadId());
}

bool
MCRWLockWriterEnqueue::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCRWLockWriterEnqueue::dependentWith(const MCTransition *other) const
{
  const MCRWLockTransition *maybeRWLockOperation =
    dynamic_cast<const MCRWLockTransition *>(other);
  if (maybeRWLockOperation) {
    return *maybeRWLockOperation->rwlock == *this->rwlock;
  }
  return false;
}

void
MCRWLockWriterEnqueue::print() const
{
  printf("thread %lu: pthread_rwlock_wrlock(%lu) (wait)\n",
         this->thread->tid, this->rwlock->getObjectId());
}