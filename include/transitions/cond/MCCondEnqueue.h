#ifndef MC_MCCONDENQUEUE_H
#define MC_MCCONDENQUEUE_H

#include "transitions/cond/MCCondTransition.h"

MCTransition *MCReadCondEnqueue(const MCSharedTransition *, void *,
                                MCStack *);

struct MCCondEnqueue : public MCCondTransition {
  std::shared_ptr<MCMutex> mutex;

public:
  MCCondEnqueue(std::shared_ptr<MCThread> running,
                std::shared_ptr<MCConditionVariable> cond,
                std::shared_ptr<MCMutex> mutex)
    : MCCondTransition(running, cond), mutex(mutex)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool countsAgainstThreadExecutionDepth() const override
  {
    return false;
  }
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // MC_MCCONDENQUEUE_H
