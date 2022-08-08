#ifndef MC_MCBARRIERWAIT_H
#define MC_MCBARRIERWAIT_H

#include "mcmini/transitions/barrier/MCBarrierTransition.h"

MCTransition *MCReadBarrierWait(const MCSharedTransition *, void *,
                                MCState *);

struct MCBarrierWait : public MCBarrierTransition {
public:
  MCBarrierWait(std::shared_ptr<MCThread> thread,
                std::shared_ptr<MCBarrier> barrier)
    : MCBarrierTransition(thread, barrier)
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

#endif // MC_MCBARRIERWAIT_H
