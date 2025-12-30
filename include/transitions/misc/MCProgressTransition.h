#ifndef MC_PROGRESS_TRANSITION_H
#define MC_PROGRESS_TRANSITION_H

#include "MCTransition.h"

/**
 * Emitted when the target program calls MC_PROGRESS() to explicitly
 * report progress. Helps detect long per-thread execution without
 * progress, a prerequisite for livelock.
 *
 * NOTE: This is purely a bookkeeping transition that
 *       should not affect thread interleavings.
 */
MCTransition *MCReadProgressTransition(const MCSharedTransition *, void *, MCStack *);

struct MCProgressTransition : public MCTransition {
public:

  MCProgressTransition(std::shared_ptr<MCThread> thread)
    : MCTransition(thread)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;

  void
  applyToState(MCStack *) override;
  bool dependentWith(const MCTransition *) const override;
  bool coenabledWith(const MCTransition *) const override;
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif
