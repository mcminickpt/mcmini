#ifndef MC_MCCONDENQUEUE_H
#define MC_MCCONDENQUEUE_H

#include "MCCondTransition.h"

MCTransition* MCReadCondEnqueue(const MCSharedTransition*, void*, MCState*);

struct MCCondEnqueue : public MCCondTransition {
    std::shared_ptr<MCMutex> mutex;
public:
    MCCondEnqueue(std::shared_ptr<MCThread> running, std::shared_ptr<MCConditionVariable> cond, std::shared_ptr<MCMutex> mutex) :
            MCCondTransition(running, cond), mutex(mutex) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool countsAgainstThreadExecutionDepth() override { return false; }
    void print() override;
};

#endif //MC_MCCONDENQUEUE_H
