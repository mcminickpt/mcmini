#ifndef MC_MCCONDWAIT_H
#define MC_MCCONDWAIT_H

#include "mcmini/transitions/cond/MCCondTransition.h"

MCTransition *MCReadCondWait(const MCSharedTransition *, void *,
                             MCState *);

/**
 * Attempts to re-acquire the mutex/exit the thread queue
 */
struct MCCondWait : public MCCondTransition {
public:
  MCCondWait(std::shared_ptr<MCThread> running,
             std::shared_ptr<MCConditionVariable> cond)
    : MCCondTransition(running, cond)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCState *) const override;
  void print() const override;
};

#endif // MC_MCCONDWAIT_H
