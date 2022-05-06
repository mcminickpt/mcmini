#ifndef GMAL_GMALCONDSIGNAL_H
#define GMAL_GMALCONDSIGNAL_H

#include "GMALCondTransition.h"

GMALTransition* GMALReadCondSignal(const GMALSharedTransition*, void*, GMALState*);

struct GMALCondSignal : public GMALCondTransition {
public:
    GMALCondSignal(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALConditionVariable> cond) :
            GMALCondTransition(running, cond) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};


#endif //GMAL_GMALCONDSIGNAL_H
