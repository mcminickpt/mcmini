#ifndef GMAL_GMALSEMPOST_H
#define GMAL_GMALSEMPOST_H

#include "GMALSemaphoreTransition.h"

GMALTransition* GMALReadSemPost(const GMALSharedTransition*, void*, GMALState*);

struct GMALSemPost : public GMALSemaphoreTransition {
public:
    GMALSemPost(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALSemaphore> sem) :
            GMALSemaphoreTransition(running, sem) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALSEMPOST_H
