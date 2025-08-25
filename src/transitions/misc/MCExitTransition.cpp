#include "transitions/misc/MCExitTransition.h"
#include "mcmini_private.h"  /* For mcprintf() */

MCTransition *
MCReadExitTransition(const MCSharedTransition *shmTransition,
                     void *shmStart, MCStack *programState)
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
MCExitTransition::dynamicCopyInState(const MCStack *state) const
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
MCExitTransition::enabledInState(const MCStack *) const
{
  return false; // Never enabled
}

MCTransitionUniqueRep
MCExitTransition::toUniqueRep() const
{
  MCTransitionUniqueRep uniqueRep;
  uniqueRep.typeId = MC_EXIT_TRANSITION;
  uniqueRep.threadId = this->thread->tid;
  uniqueRep.param.val[0] = UINT_MAX;
  uniqueRep.param.val[1] = UINT_MAX;
  return uniqueRep;
}

void
MCExitTransition::print() const
{
  mcprintf("thread %lu: exit(%u)\n", this->thread->tid, this->exitCode);
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
