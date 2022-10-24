#include "mcmini/transitions/semaphore/MCSemPost.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/semaphore/MCSemInit.h"

MCTransition *
MCReadSemPost(const MCSharedTransition *shmTransition, void *shmData,
              MCState *state)
{
  auto semInShm = static_cast<MCSemaphoreShadow *>(shmData);
  auto semThatExists =
    state->getVisibleObjectWithSystemIdentity<MCSemaphore>(
      (MCSystemID)semInShm->sem);

  // Catch undefined behavior
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    semThatExists != nullptr,
    "Attempting to post to an uninitialized semaphore");
  if (semThatExists->isDestroyed()) {
    MC_REPORT_UNDEFINED_BEHAVIOR(
      "Attempting to post to a semaphore that has been destroyed");
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCSemPost(threadThatRan, semThatExists);
}

std::shared_ptr<MCTransition>
MCSemPost::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto semCpy =
    std::static_pointer_cast<MCSemaphore, MCVisibleObject>(
      this->sem->copy());
  auto mutexInit = new MCSemPost(threadCpy, semCpy);
  return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCSemPost::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCSemaphore> semInState =
    state->getObjectWithId<MCSemaphore>(sem->getObjectId());
  auto cpy = new MCSemPost(threadInState, semInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCSemPost::applyToState(MCState *state)
{
  this->sem->post();
}

bool
MCSemPost::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCSemPost::dependentWith(const MCTransition *other) const
{
  const MCSemInit *maybeSemaphoreInitOperation =
    dynamic_cast<const MCSemInit *>(other);
  if (maybeSemaphoreInitOperation) {
    return *maybeSemaphoreInitOperation->sem == *this->sem;
  }
  return false;
}

void
MCSemPost::print() const
{
  printf("thread %lu: sem_post(%lu)\n", this->thread->tid,
         this->sem->getObjectId());
}