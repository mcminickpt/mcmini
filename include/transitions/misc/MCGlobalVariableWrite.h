#ifndef MC_MCGLOBALVARIABLEWRITE_H
#define MC_MCGLOBALVARIABLEWRITE_H

#include "transitions/misc/MCGlobalVariableTransition.h"

MCTransition *MCReadGlobalWrite(const MCSharedTransition *, void *,
                                MCStack *);

struct MCGlobalVariableWriteData {
  void *addr;
  char *varName;

  MCGlobalVariableWriteData(void *addr, char *varName)
    : addr(addr), varName(varName)
  {
  }
};

struct MCGlobalVariableWrite : public MCGlobalVariableTransition {
public:
  const void *newValue;
  MCGlobalVariableWrite(std::shared_ptr<MCThread> running,
                        std::shared_ptr<MCGlobalVariable> global)
    : MCGlobalVariableTransition(running, global)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override {}
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool isRacingWith(const MCTransition *) const override;
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // MC_MCGLOBALVARIABLEWRITE_H
