#ifndef INCLUDE_MCMINI_TRANSITIONS_MISC_MCABORTTRANSITION_HPP
#define INCLUDE_MCMINI_TRANSITIONS_MISC_MCABORTTRANSITION_HPP

#include "mcmini/MCTransition.h"

MCTransition *MCReadAbortTransition(const MCSharedTransition *,
                                    void *, MCState *);

struct MCAbortTransition : public MCTransition {
public:

  MCAbortTransition(std::shared_ptr<MCThread> thread)
    : MCTransition(thread)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;

  void
  applyToState(MCState *) override
  {}
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCState *) const override;
  bool ensuresDeadlockIsImpossible() const override;
  bool countsAgainstThreadExecutionDepth() const override;
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_MISC_MCABORTTRANSITION_HPP
