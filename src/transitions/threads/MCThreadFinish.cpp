#include "transitions/threads/MCThreadFinish.h"
#include "mcmini_private.h"  /* For mcprintf() */

MCTransition *
MCReadThreadFinish(const MCSharedTransition *shmTransition,
                   void *shmData, MCStack *state)
{
  // TODO: Potentially add asserts that the thread that just ran
  // exists!
  tid_t threadThatRanId = shmTransition->executor;
  auto threadThatRan    = state->getThreadWithId(threadThatRanId);
  return new MCThreadFinish(threadThatRan);
}

std::shared_ptr<MCTransition>
MCThreadFinish::staticCopy() const
{
  // INVARIANT: Target and the thread itself are the same
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  auto threadStartCpy = new MCThreadFinish(threadCpy);
  return std::shared_ptr<MCTransition>(threadStartCpy);
}

std::shared_ptr<MCTransition>
MCThreadFinish::dynamicCopyInState(const MCStack *state) const
{
  // INVARIANT: Target and the thread itself are the same
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  auto cpy = new MCThreadFinish(threadInState);
  return std::shared_ptr<MCTransition>(cpy);
}

void
MCThreadFinish::applyToState(MCStack *state)
{
  this->target->die();
}

void
MCThreadFinish::unapplyToState(MCStack *state)
{
  this->target->spawn();
}

bool
MCThreadFinish::isReversibleInState(const MCStack *state) const
{
  return true;
}

bool
MCThreadFinish::enabledInState(const MCStack *) const
{
  return thread->tid != TID_MAIN_THREAD;
}

bool
MCThreadFinish::coenabledWith(const MCTransition *transition) const
{
  if (this->thread->tid == transition->getThreadId()) {
    return false;
  }
  return true;
}

bool
MCThreadFinish::dependentWith(const MCTransition *transition) const
{
  if (this->thread->tid == transition->getThreadId()) { return true; }
  return false;
}

void
MCThreadFinish::print() const
{
  mcprintf("thread %lu: exits\n", this->thread->tid);
}

bool
MCThreadFinish::ensuresDeadlockIsImpossible() const
{
  return this->thread->tid == TID_MAIN_THREAD;
}

bool
MCThreadFinish::countsAgainstThreadExecutionDepth() const
{
  return false;
}
