#include "mcmini/transitions/threads/MCThreadStart.h"

MCTransition *
MCReadThreadStart(const MCSharedTransition *shmTransition,
                  void *shmData, MCState *programState)
{
  // Should never be called
  MC_FATAL();
}

std::shared_ptr<MCTransition>
MCThreadStart::staticCopy() const
{
  // INVARIANT: Target and the thread itself are the same
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  return std::make_shared<MCThreadStart>(threadCpy);
}

std::shared_ptr<MCTransition>
MCThreadStart::dynamicCopyInState(const MCState *state) const
{
  // INVARIANT: Target and the thread itself are the same
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  return std::make_shared<MCThreadStart>(threadInState);
}

void
MCThreadStart::applyToState(MCState *)
{
  this->thread->spawn();
}

void
MCThreadStart::unapplyToState(MCState *state)
{
  this->thread->despawn();
}

bool
MCThreadStart::isReversibleInState(const MCState *state) const
{
  return false;
}

bool
MCThreadStart::coenabledWith(const MCTransition *transition) const
{
  if (this->thread->tid == transition->getThreadId()) {
    return false;
  }

  // Technically, if the other transition is *never* enabled,
  // we could return false here. Such a transition is hard to
  // imagine though...
  return true;
}

bool
MCThreadStart::dependentWith(const MCTransition *transition) const
{
  return this->thread->tid == transition->getThreadId();
}

void
MCThreadStart::print() const
{
  printf("thread %lu: starts\n", this->thread->tid);
}

bool
MCThreadStart::countsAgainstThreadExecutionDepth() const
{
  return false;
}
