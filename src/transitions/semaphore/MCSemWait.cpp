#include "transitions/semaphore/MCSemWait.h"
#include "mcmini_private.h"
#include "transitions/semaphore/MCSemInit.h"

MCTransition *
MCReadSemWait(const MCSharedTransition *shmTransition, void *shmData,
              MCStack *state)
{
  auto semInShm = *static_cast<sem_t **>(shmData);
  auto semThatExists =
    state->getVisibleObjectWithSystemIdentity<MCSemaphore>(
      (MCSystemID)semInShm);

  // Catch undefined behavior
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    semThatExists != nullptr,
    "Attempting to wait on an uninitialized semaphore");
  if (semThatExists->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to wait on a semaphore that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCSemWait(threadThatRan, semThatExists);
}

std::shared_ptr<MCTransition>
MCSemWait::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto semCpy =
    std::static_pointer_cast<MCSemaphore, MCVisibleObject>(
      this->sem->copy());
  auto mutexInit = new MCSemWait(threadCpy, semCpy);
  return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCSemWait::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCSemaphore> semInState =
    state->getObjectWithId<MCSemaphore>(sem->getObjectId());
  auto cpy = new MCSemWait(threadInState, semInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCSemWait::applyToState(MCStack *state)
{
  this->sem->wait();
  this->sem->leaveWaitingQueue(this->getThreadId());
}

bool
MCSemWait::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCSemWait::dependentWith(const MCTransition *other) const
{
  const MCSemInit *maybeSemaphoreInitOperation =
    dynamic_cast<const MCSemInit *>(other);
  if (maybeSemaphoreInitOperation) {
    return *maybeSemaphoreInitOperation->sem == *this->sem;
  }

  const MCSemWait *maybeSemaphoreWaitOperation =
    dynamic_cast<const MCSemWait *>(other);
  if (maybeSemaphoreWaitOperation) {
    return *maybeSemaphoreWaitOperation->sem == *this->sem;
  }

  return false;
}

bool
MCSemWait::enabledInState(const MCStack *) const
{
  return this->sem->threadCanExit(this->getThreadId());
}

MCTransitionUniqueRep
MCSemWait::toUniqueRep() const
{
  MCTransitionUniqueRep uniqueRep;
  uniqueRep.typeId = MC_SEM_WAIT;
  uniqueRep.threadId = this->thread->tid;
  uniqueRep.param.val[0] = countVisibleObjectsOfType(this->sem->getObjectId());
  uniqueRep.param.val[1] = UINT_MAX;
  return uniqueRep;
}

void
MCSemWait::print() const
{
  mcprintf("thread %lu: sem_wait(sem:%u) (asleep -> awake)\n",
           this->thread->tid,
           countVisibleObjectsOfType(this->sem->getObjectId()));
}
