#ifndef MC_MCMUTEXLOCK_H
#define MC_MCMUTEXLOCK_H

#include "MCMutexTransition.h"

MCTransition* MCReadMutexLock(const MCSharedTransition*, void*, MCState*);

struct MCMutexLock : public MCMutexTransition {
public:

    MCMutexLock(std::shared_ptr<MCThread> thread, std::shared_ptr<MCMutex> mutex)
    : MCMutexTransition(thread, mutex) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;

    void print() override;
};

#endif //MC_MCMUTEXLOCK_H
