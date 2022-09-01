#include "mcmini/transitions/barrier/MCBarrierWait.h"

MCTransition *
MCReadBarrierWait(const MCSharedTransition *shmTransition,
                  void *shmData, MCState *state)
{
  auto barrierInShm = static_cast<MCBarrierShadow *>(shmData);
  auto barrierThatExists =
    state->getVisibleObjectWithSystemIdentity<MCBarrier>(
      (MCSystemID)barrierInShm->systemIdentity);

  // TODO: Figure out how to deal with undefined behavior
  MC_ASSERT(barrierThatExists != nullptr);

  auto executor = shmTransition->executor;
  if (!barrierThatExists->isWaitingOnBarrier(executor)) {
    barrierThatExists->wait(
      executor); // Add this thread to the waiting queue ->
                 // potentially unblocks threads waiting on the
                 // barrier
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCBarrierWait(threadThatRan, barrierThatExists);
}

std::shared_ptr<MCTransition>
MCBarrierWait::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto barrierCpy =
    std::static_pointer_cast<MCBarrier, MCVisibleObject>(
      this->barrier->copy());
  auto cpy = new MCBarrierWait(threadCpy, barrierCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCBarrierWait::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCBarrier> barrierInState =
    state->getObjectWithId<MCBarrier>(barrier->getObjectId());
  auto cpy = new MCBarrierWait(threadInState, barrierInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCBarrierWait::applyToState(MCState *state)
{
  // We don't actually need to do anything here
}

bool
MCBarrierWait::coenabledWith(const MCTransition *other) const
{
  /* We're only co-enabled if we won't guarantee block */
  return !this->barrier->wouldBlockIfWaitedOn(this->getThreadId());
}

bool
MCBarrierWait::dependentWith(const MCTransition *other) const
{
  const MCBarrierTransition *maybeBarrierOperation =
    dynamic_cast<const MCBarrierTransition *>(other);
  if (maybeBarrierOperation) {
    return *maybeBarrierOperation->barrier == *this->barrier;
  }
  return false;
}

bool
MCBarrierWait::enabledInState(const MCState *state) const
{
  return !this->barrier->wouldBlockIfWaitedOn(this->getThreadId());
}

void
MCBarrierWait::print() const
{
  printf("thread %lu: pthread_barrier_wait(%lu)\n", this->thread->tid,
         this->barrier->getObjectId());
}
