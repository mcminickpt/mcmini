#ifndef MC_MCCONDENQUEUE_H
#define MC_MCCONDENQUEUE_H

#include "mcmini/transitions/cond/MCCondTransition.h"

MCTransition *MCReadCondEnqueue(const MCSharedTransition *, void *,
                                MCState *);

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
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool countsAgainstThreadExecutionDepth() const override
  {
    return false;
  }
  void print() const override;
};

#endif // MC_MCCONDENQUEUE_H
