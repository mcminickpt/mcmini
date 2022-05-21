#ifndef MC_MCMUTEXUNLOCK_H
#define MC_MCMUTEXUNLOCK_H

#include "MCMutexTransition.h"
#include <memory>

MCTransition* MCReadMutexUnlock(const MCSharedTransition*, void*, MCState*);

struct MCMutexUnlock : public MCMutexTransition {
public:

    MCMutexUnlock(std::shared_ptr<MCThread> thread, std::shared_ptr<MCMutex> mutex) : MCMutexTransition(thread, mutex) {}
    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;

    void print() override;
};

#endif //MC_MCMUTEXUNLOCK_H
