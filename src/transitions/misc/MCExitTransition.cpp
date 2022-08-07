#include "mcmini/transitions/misc/MCExitTransition.h"

MCTransition *
MCReadExitTransition(const MCSharedTransition *shmTransition,
                     void *shmStart, MCState *programState)
{
  auto executor =
    programState->getThreadWithId(shmTransition->executor);
  auto exitCode = *(int *)shmStart;
  return new MCExitTransition(executor, exitCode);
}

std::shared_ptr<MCTransition>
MCExitTransition::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto threadStartCpy = new MCExitTransition(threadCpy, exitCode);
  return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCExitTransition::dynamicCopyInState(const MCState *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  auto cpy = new MCExitTransition(threadInState, exitCode);
  return std::shared_ptr<MCTransition>(cpy);
}

bool
MCExitTransition::dependentWith(const MCTransition *) const
{
  return false;
}

bool
MCExitTransition::enabledInState(const MCState *) const
{
  return false; // Never enabled
}

void
MCExitTransition::print() const
{
  printf("thread %lu: exit(%u)\n", this->thread->tid, this->exitCode);
}

bool
MCExitTransition::ensuresDeadlockIsImpossible() const
{
  return true;
}

bool
MCExitTransition::countsAgainstThreadExecutionDepth() const
{
  return false;
}
