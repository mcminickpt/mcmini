#include "mcmini/transitions/cond/MCCondInit.h"

MCTransition *
MCReadCondInit(const MCSharedTransition *shmTransition, void *shmData,
               MCState *state)
{
  auto condInShm = static_cast<pthread_cond_t **>(shmData);
  auto systemId  = (MCSystemID)*condInShm;
  auto condThatExists =
    state->getVisibleObjectWithSystemIdentity<MCConditionVariable>(
      systemId);

  if (condThatExists == nullptr) {
    auto shadow  = MCConditionVariableShadow(*condInShm);
    auto newCond = new MCConditionVariable(shadow);
    auto newMutexSharedPtr =
      std::shared_ptr<MCVisibleObject>(newCond);
    state->registerVisibleObjectWithSystemIdentity(systemId,
                                                   newMutexSharedPtr);
    condThatExists =
      std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
        newMutexSharedPtr);
  }

  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCCondInit(threadThatRan, condThatExists);
}

std::shared_ptr<MCTransition>
MCCondInit::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto condCpy =
    std::static_pointer_cast<MCConditionVariable, MCVisibleObject>(
      this->conditionVariable->copy());
  auto condInit = new MCCondInit(threadCpy, condCpy);
  return std::shared_ptr<MCTransition>(condInit);
}

std::shared_ptr<MCTransition>
MCCondInit::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  std::shared_ptr<MCConditionVariable> mutexInState =
    state->getObjectWithId<MCConditionVariable>(
      conditionVariable->getObjectId());
  auto cpy = new MCCondInit(threadInState, mutexInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCCondInit::applyToState(MCState *state)
{
  this->conditionVariable->initialize();
}

bool
MCCondInit::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCCondInit::dependentWith(const MCTransition *other) const
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
MCCondInit::print() const
{
  printf("thread %lu: pthread_cond_init(%lu)\n", this->thread->tid,
         this->conditionVariable->getObjectId());
}