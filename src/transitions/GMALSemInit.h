#ifndef GMAL_GMALSEMINIT_H
#define GMAL_GMALSEMINIT_H

#include "GMALSemaphoreTransition.h"

GMALTransition* GMALReadSemInit(const GMALSharedTransition*, void*, GMALState*);

struct GMALSemInit : public GMALSemaphoreTransition {
public:
    GMALSemInit(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALSemaphore> sem) :
            GMALSemaphoreTransition(running, sem) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALSEMINIT_H
