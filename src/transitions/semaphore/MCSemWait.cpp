#include "mcmini/transitions/semaphore/MCSemWait.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/semaphore/MCSemInit.h"

MCTransition *
MCReadSemWait(const MCSharedTransition *shmTransition, void *shmData,
              MCState *state)
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
MCSemWait::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCSemaphore> semInState =
    state->getObjectWithId<MCSemaphore>(sem->getObjectId());
  auto cpy = new MCSemWait(threadInState, semInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCSemWait::applyToState(MCState *state)
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
MCSemWait::enabledInState(const MCState *) const
{
  return this->sem->threadCanExit(this->getThreadId());
}

void
MCSemWait::print() const
{
  printf("thread %lu: sem_wait(%lu) (asleep -> awake)\n",
         this->thread->tid, this->sem->getObjectId());
}
