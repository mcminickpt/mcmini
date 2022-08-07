#ifndef MC_MCBARRIERENQUEUE_H
#define MC_MCBARRIERENQUEUE_H

#include "mcmini/transitions/barrier/MCBarrierTransition.h"

MCTransition *MCReadBarrierEnqueue(const MCSharedTransition *, void *,
                                   MCState *);

struct MCBarrierEnqueue : public MCBarrierTransition {
public:
  MCBarrierEnqueue(std::shared_ptr<MCThread> thread,
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
  bool countsAgainstThreadExecutionDepth() const override
  {
    return false;
  }
  void print() const override;
};

#endif // MC_MCBARRIERENQUEUE_H
