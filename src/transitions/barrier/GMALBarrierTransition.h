#ifndef GMAL_GMALBARRIERTRANSITION_H
#define GMAL_GMALBARRIERTRANSITION_H

#include "GMALTransition.h"
#include "objects/GMALBarrier.h"

struct GMALBarrierTransition : public GMALTransition {
public:
    std::shared_ptr<GMALBarrier> barrier;
    GMALBarrierTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALBarrier> barrier) :
            GMALTransition(running), barrier(barrier) {}
};

#endif //GMAL_GMALBARRIERTRANSITION_H
