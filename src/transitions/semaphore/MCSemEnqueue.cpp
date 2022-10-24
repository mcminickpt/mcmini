#include "mcmini/transitions/semaphore/MCSemEnqueue.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/semaphore/MCSemInit.h"
#include "mcmini/transitions/semaphore/MCSemWait.h"

MCTransition *
MCReadSemEnqueue(const MCSharedTransition *shmTransition,
                 void *shmData, MCState *state)
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
  return new MCSemEnqueue(threadThatRan, semThatExists);
}

std::shared_ptr<MCTransition>
MCSemEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto semCpy =
    std::static_pointer_cast<MCSemaphore, MCVisibleObject>(
      this->sem->copy());
  auto mutexInit = new MCSemEnqueue(threadCpy, semCpy);
  return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCSemEnqueue::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCSemaphore> semInState =
    state->getObjectWithId<MCSemaphore>(sem->getObjectId());
  auto cpy = new MCSemEnqueue(threadInState, semInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCSemEnqueue::applyToState(MCState *state)
{
  this->sem->enterWaitingQueue(this->getThreadId());
}

bool
MCSemEnqueue::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCSemEnqueue::dependentWith(const MCTransition *other) const
{
  const MCSemInit *maybeSemaphoreInitOperation =
    dynamic_cast<const MCSemInit *>(other);
  if (maybeSemaphoreInitOperation) {
    return *maybeSemaphoreInitOperation->sem == *this->sem;
  }

  // The enqueue operation is only dependent with a sem_wait
  // when the semaphore wakes threads using LIFO
  // auto maybeSemaphoreWaitOperation =
  // std::dynamic_pointer_cast<MCSemInit, MCTransition>(other); if
  // (maybeSemaphoreWaitOperation) {
  //    return *maybeSemaphoreWaitOperation->sem == *this->sem;
  //}

  const MCSemEnqueue *maybeSemaphoreEnqueueOperation =
    dynamic_cast<const MCSemEnqueue *>(other);
  if (maybeSemaphoreEnqueueOperation) {
    return *maybeSemaphoreEnqueueOperation->sem == *this->sem;
  }

  return false;
}

void
MCSemEnqueue::print() const
{
  printf("thread %lu: sem_wait(%lu) (awake -> asleep)\n",
         this->thread->tid, this->sem->getObjectId());
}
