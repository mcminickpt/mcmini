#ifndef MC_MCCONDSIGNAL_H
#define MC_MCCONDSIGNAL_H

#include "MCCondTransition.h"

MCTransition* MCReadCondSignal(const MCSharedTransition*, void*, MCState*);

struct MCCondSignal : public MCCondTransition {
public:
    MCCondSignal(std::shared_ptr<MCThread> running, std::shared_ptr<MCConditionVariable> cond) :
            MCCondTransition(running, cond) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};


#endif //MC_MCCONDSIGNAL_H
