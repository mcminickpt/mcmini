#ifndef MC_MCBARRIERINIT_H
#define MC_MCBARRIERINIT_H

#include "mcmini/transitions/barrier/MCBarrierTransition.h"

MCTransition *MCReadBarrierInit(const MCSharedTransition *, void *,
                                MCState *);

struct MCBarrierInit : public MCBarrierTransition {
public:
  MCBarrierInit(std::shared_ptr<MCThread> thread,
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
  void print() const override;
};

#endif // MC_MCBARRIERINIT_H
