#ifndef MC_MCTHREADREACHGOAL_H
#define MC_MCTHREADREACHGOAL_H

#include "MCThreadTransition.h"

MCTransition* MCReadThreadReachGoal(const MCSharedTransition*, void*, MCState*);

struct MCThreadReachGoal : MCThreadTransition {
    inline explicit MCThreadReachGoal(std::shared_ptr<MCThread> thread) : MCThreadTransition(thread) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool countsAgainstThreadExecutionDepth() override;
    void print() override;
};

#endif //MC_MCTHREADREACHGOAL_H
