#ifndef GMAL_GMALBARRIERWAIT_H
#define GMAL_GMALBARRIERWAIT_H

#include "GMALBarrierTransition.h"

GMALTransition* GMALReadBarrierWait(const GMALSharedTransition*, void*, GMALState*);

struct GMALBarrierWait : public GMALBarrierTransition {
public:
    GMALBarrierWait(std::shared_ptr<GMALThread> thread, std::shared_ptr<GMALBarrier> barrier)
            : GMALBarrierTransition(thread, barrier) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool enabledInState(const GMALState *) override;
    void print() override;
};

#endif //GMAL_GMALBARRIERWAIT_H
