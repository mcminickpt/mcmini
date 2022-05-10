#ifndef GMAL_GMALMUTEXLOCK_H
#define GMAL_GMALMUTEXLOCK_H

#include "GMALMutexTransition.h"

GMALTransition* GMALReadMutexLock(const GMALSharedTransition*, void*, GMALState*);

struct GMALMutexLock : public GMALMutexTransition {
public:

    GMALMutexLock(std::shared_ptr<GMALThread> thread, std::shared_ptr<GMALMutex> mutex)
    : GMALMutexTransition(thread, mutex) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool enabledInState(const GMALState *) override;

    void print() override;
};

#endif //GMAL_GMALMUTEXLOCK_H
