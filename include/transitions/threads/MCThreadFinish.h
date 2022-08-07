#ifndef MC_MCTHREADFINISH_H
#define MC_MCTHREADFINISH_H

#include "mcmini/transitions/threads/MCThreadTransition.h"

MCTransition *
MCReadThreadFinish(const MCSharedTransition *shmTransition,
                   void *shmData, MCState *state);

struct MCThreadFinish : public MCThreadTransition {
public:
  inline explicit MCThreadFinish(
    std::shared_ptr<MCThread> threadRunning)
    : MCThreadTransition(threadRunning, threadRunning)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  void unapplyToState(MCState *) override;
  bool isReversibleInState(const MCState *) const override;
  bool enabledInState(const MCState *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool ensuresDeadlockIsImpossible() const override;
  bool countsAgainstThreadExecutionDepth() const override;
  void print() const override;
};

#endif // MC_MCTHREADFINISH_H
