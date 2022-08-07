#include "mcmini/transitions/misc/MCGlobalVariableWrite.h"

MCTransition *
MCReadGlobalWrite(const MCSharedTransition *shmTransition,
                  void *shmStart, MCState *state)
{
  auto data = *(MCGlobalVariableWriteData *)shmStart;
  auto threadThatRan =
    state->getThreadWithId(shmTransition->executor);
  auto globalVariable =
    state->getVisibleObjectWithSystemIdentity<MCGlobalVariable>(
      data.addr);

  /* New global variable */
  if (globalVariable == nullptr) {
    globalVariable = std::make_shared<MCGlobalVariable>(data.addr);
    state->registerVisibleObjectWithSystemIdentity(data.addr,
                                                   globalVariable);
  }

  return new MCGlobalVariableWrite(threadThatRan, globalVariable,
                                   data.newValue);
}

std::shared_ptr<MCTransition>
MCGlobalVariableWrite::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto globalCpy =
    std::static_pointer_cast<MCGlobalVariable, MCVisibleObject>(
      this->global->copy());
  auto newValueCpy = (void *)this->newValue;
  return std::make_shared<MCGlobalVariableWrite>(threadCpy, globalCpy,
                                                 newValueCpy);
}

std::shared_ptr<MCTransition>
MCGlobalVariableWrite::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  auto globalInState =
    state->getObjectWithId<MCGlobalVariable>(global->getObjectId());

  // TODO: Verify if copying the value directly instead of storing
  // with the associated object is correct
  auto newValueCpy = (void *)this->newValue;
  return std::make_shared<MCGlobalVariableWrite>(
    threadInState, globalInState, newValueCpy);
}

bool
MCGlobalVariableWrite::coenabledWith(const MCTransition *) const
{
  return true; /* Co-enabled with anything else */
}

bool
MCGlobalVariableWrite::dependentWith(
  const MCTransition *transition) const
{
  const MCGlobalVariableTransition *maybeGlobalVariableAccess =
    dynamic_cast<const MCGlobalVariableTransition *>(transition);
  if (maybeGlobalVariableAccess != nullptr) {
    return *this->global == *maybeGlobalVariableAccess->global;
  }
  return false;
}

bool
MCGlobalVariableWrite::isRacingWith(
  const MCTransition *transition) const
{
  const MCGlobalVariableTransition *maybeGlobalVariableAccess =
    dynamic_cast<const MCGlobalVariableTransition *>(transition);
  if (maybeGlobalVariableAccess != nullptr) {
    return *this->global == *maybeGlobalVariableAccess->global;
  }
  return false;
}

void
MCGlobalVariableWrite::print() const
{
  printf("thread %lu: WRITE(%p, %p)\n", this->thread->tid,
         this->global->addr, this->newValue);
}
