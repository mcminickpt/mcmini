#ifndef GMAL_GMALSEMENQUEUE_H
#define GMAL_GMALSEMENQUEUE_H

#include "GMALSemaphoreTransition.h"

GMALTransition* GMALReadSemEnqueue(const GMALSharedTransition*, void*, GMALState*);

struct GMALSemEnqueue : public GMALSemaphoreTransition {
public:
    GMALSemEnqueue(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALSemaphore> sem) :
            GMALSemaphoreTransition(running, sem) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALSEMENQUEUE_H
