#ifndef MC_MCCONDSIGNAL_H
#define MC_MCCONDSIGNAL_H

#include "transitions/cond/MCCondTransition.h"

MCTransition *MCReadCondSignal(const MCSharedTransition *, void *,
                               MCStack *);

struct MCCondSignal : public MCCondTransition {
public:
  MCCondSignal(std::shared_ptr<MCThread> running,
               std::shared_ptr<MCConditionVariable> cond)
    : MCCondTransition(running, cond)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // MC_MCCONDSIGNAL_H
