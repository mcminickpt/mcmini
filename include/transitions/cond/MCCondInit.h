#ifndef MC_MCCONDINIT_H
#define MC_MCCONDINIT_H

#include "transitions/cond/MCCondTransition.h"

MCTransition *MCReadCondInit(const MCSharedTransition *, void *,
                             MCStack *);

struct MCCondInit : public MCCondTransition {
public:
  MCCondInit(std::shared_ptr<MCThread> thread,
             std::shared_ptr<MCConditionVariable> cond)
    : MCCondTransition(thread, cond)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // MC_MCCONDINIT_H
