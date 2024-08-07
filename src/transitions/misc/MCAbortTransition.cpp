#include "transitions/misc/MCAbortTransition.h"
#include "mcmini_private.h"  /* For mcprintf() */

MCTransition *
MCReadAbortTransition(const MCSharedTransition *shmTransition,
                      void *shmStart, MCStack *programState)
{
  auto executor =
    programState->getThreadWithId(shmTransition->executor);
  return new MCAbortTransition(executor);
}

std::shared_ptr<MCTransition>
MCAbortTransition::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto threadStartCpy = new MCAbortTransition(threadCpy);
  return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCAbortTransition::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  auto cpy = new MCAbortTransition(threadInState);
  return std::shared_ptr<MCTransition>(cpy);
}

bool
MCAbortTransition::dependentWith(const MCTransition *) const
{
  return false;
}

bool
MCAbortTransition::enabledInState(const MCStack *) const
{
  // We allow abort() calls to run to completion.
  // This way, when the true abort() function is invoked,
  // the transition turns up in a backtrace
  return true;
}

void
MCAbortTransition::print() const
{
  mcprintf("thread %lu: abort()\n", this->thread->tid);
}

bool
MCAbortTransition::ensuresDeadlockIsImpossible() const
{
  // A thread could always call abort() to break any deadlock among
  // the other threads
  return true;
}

bool
MCAbortTransition::countsAgainstThreadExecutionDepth() const
{
  return false;
}
