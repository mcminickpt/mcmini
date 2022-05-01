#ifndef GMAL_GMALCONDWAIT_H
#define GMAL_GMALCONDWAIT_H

#include "GMALCondTransition.h"

GMALTransition* GMALReadCondWait(const GMALSharedTransition*, void*, GMALState*);

struct GMALCondWait : public GMALCondTransition {
public:
    GMALCondWait(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALConditionVariable> cond) :
            GMALCondTransition(running, cond) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool enabledInState(const GMALState *) override;
    void print() override;
};

#endif //GMAL_GMALCONDWAIT_H
