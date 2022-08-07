#include "mcmini/transitions/cond/MCCondBroadcast.h"
#include "mcmini/MCMINI_Private.h"
#include "mcmini/transitions/mutex/MCMutexTransition.h"

MCTransition *
MCReadCondBroadcast(const MCSharedTransition *shmTransition,
                    void *shmData, MCState *state)
{
  const auto condInShm    = static_cast<pthread_cond_t **>(shmData);
  const auto condSystemId = (MCSystemID)*condInShm;
  const auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      condSystemId);

  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    condThatExists != nullptr, "Attempting to signal a condition "
                               "variable that is uninitialized");
  MC_REPORT_UNDEFINED_BEHAVIOR_ON_FAIL(
    !condThatExists->isDestroyed(),
    "Attempting to signal a destroyed condition variable");

  const auto threadThatRanId = shmTransition->executor;
  auto threadThatRan = state->getThreadWithId(threadThatRanId);
  return new MCCondBroadcast(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondBroadcast::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto cpy = new MCCondBroadcast(threadCpy, condCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondBroadcast::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> condInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  auto cpy = new MCCondBroadcast(threadInState, condInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondBroadcast::applyToState(MCState *state)
{
  this->conditionVariable->wakeAllSleepingThreads();
}

bool
MCCondBroadcast::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCCondBroadcast::dependentWith(const MCTransition *other) const
{
  const MCCondTransition *maybeCondOperation =
    dynamic_cast<const MCCondTransition *>(other);
  if (maybeCondOperation) {
    return *maybeCondOperation->conditionVariable ==
           *this->conditionVariable;
  }
  return false;
}

void
MCCondBroadcast::print() const
{
  printf("thread %lu: pthread_cond_broadcast(%lu)\n",
         this->thread->tid, this->conditionVariable->getObjectId());
}
