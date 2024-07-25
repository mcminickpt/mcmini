#ifndef MC_MCBARRIERTRANSITION_H
#define MC_MCBARRIERTRANSITION_H

#include "mcmini/MCTransition.h"
#include "mcmini/objects/MCBarrier.h"

struct MCBarrierTransition : public MCTransition {
public:
  std::shared_ptr<MCBarrier> barrier;
  MCBarrierTransition(std::shared_ptr<MCThread> running,
                      std::shared_ptr<MCBarrier> barrier)
    : MCTransition(running), barrier(barrier)
  {
  }
};

#endif // MC_MCBARRIERTRANSITION_H
