#ifndef GMAL_GMALSEMWAIT_H
#define GMAL_GMALSEMWAIT_H

#include "GMALSemaphoreTransition.h"

GMALTransition* GMALReadSemWait(const GMALSharedTransition*, void*, GMALState*);

struct GMALSemWait : public GMALSemaphoreTransition {
public:
    GMALSemWait(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALSemaphore> sem) :
            GMALSemaphoreTransition(running, sem) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool enabledInState(const GMALState *) override;
    void print() override;
};


#endif //GMAL_GMALSEMWAIT_H
