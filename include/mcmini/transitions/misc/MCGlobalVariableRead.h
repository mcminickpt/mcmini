#ifndef MC_MCGLOBALVARIABLEREAD_H
#define MC_MCGLOBALVARIABLEREAD_H

#include "mcmini/transitions/misc/MCGlobalVariableTransition.h"

MCTransition *MCReadGlobalRead(const MCSharedTransition *, void *,
                               MCStack *);

struct MCGlobalVariableRead : public MCGlobalVariableTransition {
public:

  MCGlobalVariableRead(std::shared_ptr<MCThread> running,
                       std::shared_ptr<MCGlobalVariable> global)
    : MCGlobalVariableTransition(running, global)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void
  applyToState(MCStack *) override
  {}
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool isRacingWith(const MCTransition *) const override;
  void print() const override;
};

#endif // MC_MCGLOBALVARIABLEREAD_H
