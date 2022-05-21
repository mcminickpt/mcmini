#ifndef MC_MCMUTEXINIT_H
#define MC_MCMUTEXINIT_H

#include "MCMutexTransition.h"

MCTransition* MCReadMutexInit(const MCSharedTransition*, void*, MCState*);

struct MCMutexInit : public MCMutexTransition {
public:

    MCMutexInit(std::shared_ptr<MCThread> thread, std::shared_ptr<MCMutex> mutex)
    : MCMutexTransition(thread, mutex) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCMUTEXINIT_H
