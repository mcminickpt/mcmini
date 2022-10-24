#include "mcmini/transitions/cond/MCCondSignal.h"
#include "mcmini/mcmini_private.h"
#include "mcmini/transitions/mutex/MCMutexTransition.h"

MCTransition *
MCReadCondSignal(const MCSharedTransition *shmTransition,
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
  return new MCCondSignal(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondSignal::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto cpy = new MCCondSignal(threadCpy, condCpy);
  return std::shared_ptr<MCTransition>(cpy);
}

std::shared_ptr<MCTransition>
MCCondSignal::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> condInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  auto cpy = new MCCondSignal(threadInState, condInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondSignal::applyToState(MCState *state)
{
  this->conditionVariable->sendSignalMessage();
}

bool
MCCondSignal::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCCondSignal::dependentWith(const MCTransition *other) const
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
MCCondSignal::print() const
{
  printf("thread %lu: pthread_cond_signal(%lu)\n", this->thread->tid,
         this->conditionVariable->getObjectId());
}
