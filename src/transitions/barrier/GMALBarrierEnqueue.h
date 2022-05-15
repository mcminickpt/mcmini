#ifndef GMAL_GMALBARRIERENQUEUE_H
#define GMAL_GMALBARRIERENQUEUE_H

#include "GMALBarrierTransition.h"

GMALTransition* GMALReadBarrierEnqueue(const GMALSharedTransition*, void*, GMALState*);

struct GMALBarrierEnqueue : public GMALBarrierTransition {
public:
    GMALBarrierEnqueue(std::shared_ptr<GMALThread> thread, std::shared_ptr<GMALBarrier> barrier)
            : GMALBarrierTransition(thread, barrier) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALBARRIERENQUEUE_H
