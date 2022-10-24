#include "mcmini/transitions/barrier/MCBarrierEnqueue.h"
#include "mcmini/mcmini_private.h"

MCTransition *
MCReadBarrierEnqueue(const MCSharedTransition *shmTransition,
                     void *shmData, MCState *state)
{
  auto barrierInShm = static_cast<MCBarrierShadow *>(shmData);
  auto barrierThatExists =
    state->getVisibleObjectWithSystemIdentity<MCBarrier>(
      (MCSystemID)barrierInShm->systemIdentity);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    barrierThatExists != nullptr,
    "Attempting to wait on a barrier that hasn't been initialized");

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCBarrierEnqueue(threadThatRan, barrierThatExists);
}

std::shared_ptr<MCTransition>
MCBarrierEnqueue::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto barrierCpy =
    std::static_pointer_cast<MCBarrier, MCVisibleObject>(
      this->barrier->copy());
  auto cpy = new MCBarrierEnqueue(threadCpy, barrierCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCBarrierEnqueue::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCBarrier> barrierInState =
    state->getObjectWithId<MCBarrier>(barrier->getObjectId());
  auto cpy = new MCBarrierEnqueue(threadInState, barrierInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCBarrierEnqueue::applyToState(MCState *state)
{
  auto executor = this->getThreadId();
  barrier->wait(
    executor); // Add this thread to the waiting queue -> potentially
               // unblocks threads waiting on the barrier
}

bool
MCBarrierEnqueue::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCBarrierEnqueue::dependentWith(const MCTransition *other) const
{
  const MCBarrierTransition *maybeBarrierOperation =
    dynamic_cast<const MCBarrierTransition *>(other);
  if (maybeBarrierOperation) {
    return *maybeBarrierOperation->barrier == *this->barrier;
  }
  return false;
}

void
MCBarrierEnqueue::print() const
{
  printf("thread %lu: pthread_barrier_wait(%lu) (enqueue)\n",
         this->thread->tid, this->barrier->getObjectId());
}
