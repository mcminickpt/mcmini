#ifndef GMAL_GMALMUTEXINIT_H
#define GMAL_GMALMUTEXINIT_H

#include "GMALMutexTransition.h"

GMALTransition* GMALReadMutexInit(const GMALSharedTransition*, void*, GMALState*);

struct GMALMutexInit : public GMALMutexTransition {
public:

    GMALMutexInit(std::shared_ptr<GMALThread> thread, std::shared_ptr<GMALMutex> mutex)
    : GMALMutexTransition(thread, mutex) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALMUTEXINIT_H
