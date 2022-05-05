#ifndef GMAL_GMALBARRIERINIT_H
#define GMAL_GMALBARRIERINIT_H

#include "GMALBarrierTransition.h"

GMALTransition* GMALReadBarrierInit(const GMALSharedTransition*, void*, GMALState*);

struct GMALBarrierInit : public GMALBarrierTransition {
public:
    GMALBarrierInit(std::shared_ptr<GMALThread> thread, std::shared_ptr<GMALBarrier> barrier)
    : GMALBarrierTransition(thread, barrier) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALBARRIERINIT_H
