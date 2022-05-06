#ifndef GMAL_GMALCONDENQUEUE_H
#define GMAL_GMALCONDENQUEUE_H

#include "GMALCondTransition.h"

GMALTransition* GMALReadCondEnqueue(const GMALSharedTransition*, void*, GMALState*);

struct GMALCondEnqueue : public GMALCondTransition {
    std::shared_ptr<GMALMutex> mutex;
public:
    GMALCondEnqueue(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALConditionVariable> cond, std::shared_ptr<GMALMutex> mutex) :
            GMALCondTransition(running, cond), mutex(mutex) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;

    bool countsAgainstThreadExecutionDepth() override { return false; }
};

#endif //GMAL_GMALCONDENQUEUE_H
