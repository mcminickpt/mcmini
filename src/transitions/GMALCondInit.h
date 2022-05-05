#ifndef GMAL_GMALCONDINIT_H
#define GMAL_GMALCONDINIT_H

#include "GMALCondTransition.h"

GMALTransition* GMALReadCondInit(const GMALSharedTransition*, void*, GMALState*);

struct GMALCondInit : public GMALCondTransition {
public:
    GMALCondInit(std::shared_ptr<GMALThread> thread, std::shared_ptr<GMALConditionVariable> cond)
            : GMALCondTransition(thread, cond) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALCONDINIT_H
