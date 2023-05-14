#ifndef MC_MCEXITTRANSITION_H
#define MC_MCEXITTRANSITION_H

#include "mcmini/MCTransition.h"

MCTransition *MCReadExitTransition(const MCSharedTransition *, void *,
                                   MCStack *);

struct MCExitTransition : public MCTransition {
private:

  const int exitCode;

public:

  MCExitTransition(std::shared_ptr<MCThread> thread, int exitCode)
    : MCTransition(thread), exitCode(exitCode)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;

  void
  applyToState(MCStack *) override
  {}
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCStack *) const override;
  bool ensuresDeadlockIsImpossible() const override;
  bool countsAgainstThreadExecutionDepth() const override;
  void print() const override;
};

#endif // MC_MCEXITTRANSITION_H
