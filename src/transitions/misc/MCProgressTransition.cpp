#include "transitions/misc/MCProgressTransition.h"
#include "mcmini_private.h"

MCTransition*
MCReadProgressTransition(const MCSharedTransition *shm,
                         void *, MCStack *state)
{
  auto thread = state->getThreadWithId(shm->executor);
  return new MCProgressTransition(thread);
}

std::shared_ptr<MCTransition>
MCProgressTransition::staticCopy() const
{
  auto threadCpy =
    std::static_pointer_cast<MCThread, MCVisibleObject>(
      this->thread->copy());
  return std::make_shared<MCProgressTransition>(threadCpy);
}

std::shared_ptr<MCTransition>
MCProgressTransition::dynamicCopyInState(const MCStack *state) const
{
  std::shared_ptr<MCThread> threadInState =
    state->getThreadWithId(thread->tid);
  return std::make_shared<MCProgressTransition>(threadInState);
}

void
MCProgressTransition::applyToState(MCStack *state)
{
  state->resetNoProgressCount(this->thread->tid);
}

bool
MCProgressTransition::coenabledWith(const MCTransition *other) const
{
  return true;
}

bool
MCProgressTransition::dependentWith(const MCTransition *other) const
{
  return false;
}

MCTransitionUniqueRep
MCProgressTransition::toUniqueRep() const
{
  MCTransitionUniqueRep uniqueRep;
  uniqueRep.typeId = MC_PROGRESS_TRANSITION;
  uniqueRep.threadId = this->thread->tid;
  uniqueRep.param.val[0] = UINT_MAX;
  uniqueRep.param.val[1] = UINT_MAX;
  return uniqueRep;
}

void
MCProgressTransition::print() const
{
  mcprintf("thread %lu: PROGRESS\n", thread->tid);
}
