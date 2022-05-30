#ifndef MCMINI_MCTHREADREQUESTNEWGOAL_H
#define MCMINI_MCTHREADREQUESTNEWGOAL_H

#include "MCThreadTransition.h"

MCTransition* MCReadThreadRequestNewGoal(const MCSharedTransition*, void*, MCState*);

struct MCThreadRequestNewGoal : MCThreadTransition {
    inline explicit MCThreadRequestNewGoal(std::shared_ptr<MCThread> thread) : MCThreadTransition(thread) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;

    bool countsAgainstThreadExecutionDepth() override;
    void print() override;
};

#endif //MCMINI_MCTHREADREQUESTNEWGOAL_H
