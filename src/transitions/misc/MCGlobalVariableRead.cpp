#include "mcmini/transitions/misc/MCGlobalVariableRead.h"
#include "mcmini/transitions/misc/MCGlobalVariableWrite.h"

MCTransition *
MCReadGlobalRead(const MCSharedTransition *shmTransition,
                 void *shmStart, MCState *state)
{
  auto addr = *(void **)shmStart;
  auto threadThatRan =
    state->getThreadWithId(shmTransition->executor);
  auto globalVariable =
    state->getVisibleObjectWithSystemIdentity<MCGlobalVariable>(addr);

  /* New global variable */
  if (globalVariable == nullptr) {
    globalVariable = std::make_shared<MCGlobalVariable>(addr);
    state->registerVisibleObjectWithSystemIdentity(addr,
                                                   globalVariable);
  }

  return new MCGlobalVariableRead(threadThatRan, globalVariable);
}

std::shared_ptr<MCTransition>
MCGlobalVariableRead::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto globalCpy =
    std::static_pointer_cast<MCGlobalVariable, MCVisibleObject>(
      this->global->copy());
  return std::make_shared<MCGlobalVariableRead>(threadCpy, globalCpy);
}

std::shared_ptr<MCTransition>
MCGlobalVariableRead::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  auto globalInState =
    state->getObjectWithId<MCGlobalVariable>(global->getObjectId());
  return std::make_shared<MCGlobalVariableRead>(threadInState,
                                                globalInState);
}

bool
MCGlobalVariableRead::coenabledWith(const MCTransition *) const
{
  return true; /* Co-enabled with anything else */
}

bool
MCGlobalVariableRead::dependentWith(
  const MCTransition *transition) const
{
  const MCGlobalVariableWrite *maybeGlobalVariableWrite =
    dynamic_cast<const MCGlobalVariableWrite *>(transition);
  if (maybeGlobalVariableWrite != nullptr) {
    return *this->global == *maybeGlobalVariableWrite->global;
  }
  return false;
}

bool
MCGlobalVariableRead::isRacingWith(
  const MCTransition *transition) const
{
  const MCGlobalVariableWrite *maybeGlobalVariableWrite =
    dynamic_cast<const MCGlobalVariableWrite *>(transition);
  if (maybeGlobalVariableWrite != nullptr) {
    return *this->global == *maybeGlobalVariableWrite->global;
  }
  return false;
}

void
MCGlobalVariableRead::print() const
{
  printf("thread %lu: READ(%p)\n", this->thread->tid,
         this->global->addr);
}
