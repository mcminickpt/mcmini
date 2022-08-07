#include "mcmini/transitions/barrier/MCBarrierInit.h"

MCTransition *
MCReadBarrierInit(const MCSharedTransition *shmTransition,
                  void *shmData, MCState *state)
{
  auto barrierInShm = static_cast<MCBarrierShadow *>(shmData);
  auto systemId     = (MCSystemID)barrierInShm->systemIdentity;
  auto barrierThatExists =
    state->getVisibleObjectWithSystemIdentity<MCBarrier>(systemId);

  if (barrierThatExists == nullptr) {
    auto newBarrier = new MCBarrier(*barrierInShm);
    auto newBarrierSharedPtr =
      std::shared_ptr<MCVisibleObject>(newBarrier);
    state->registerVisibleObjectWithSystemIdentity(
      systemId, newBarrierSharedPtr);
    barrierThatExists =
      std::static_pointer_cast<MCBarrier, MCVisibleObject>(
        newBarrierSharedPtr);
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCBarrierInit(threadThatRan, barrierThatExists);
}

std::shared_ptr<MCTransition>
MCBarrierInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto mutexCpy =
    std::static_pointer_cast<MCBarrier, MCVisibleObject>(
      this->barrier->copy());
  auto mutexInit = new MCBarrierInit(threadCpy, mutexCpy);
  return std::shared_ptr<MCTransition>(mutexInit);
}

std::shared_ptr<MCTransition>
MCBarrierInit::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCBarrier> mutexInState =
    state->getObjectWithId<MCBarrier>(barrier->getObjectId());
  auto cpy = new MCBarrierInit(threadInState, mutexInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCBarrierInit::applyToState(MCState *state)
{
  this->barrier->init();
}

bool
MCBarrierInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCBarrierInit::dependentWith(const MCTransition *other) const
{
  const MCBarrierTransition *maybeBarrierOperation =
    dynamic_cast<const MCBarrierTransition *>(other);
  if (maybeBarrierOperation) {
    return *maybeBarrierOperation->barrier == *this->barrier;
  }
  return false;
}

void
MCBarrierInit::print() const
{
  printf("thread %lu: pthread_barrier_init(%lu, _, %u)\n",
         this->thread->tid, this->barrier->getObjectId(),
         this->barrier->getCount());
}