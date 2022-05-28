#ifndef MCMINI_MCTHREADENTERGOALCRITICALSECTION_H
#define MCMINI_MCTHREADENTERGOALCRITICALSECTION_H

#include "MCThreadTransition.h"

MCTransition* MCReadThreadEnterGoalCriticalSection(const MCSharedTransition*, void*, MCState*);

struct MCThreadEnterGoalCriticalSection : MCThreadTransition {
    inline explicit MCThreadEnterGoalCriticalSection(std::shared_ptr<MCThread> thread) : MCThreadTransition(thread) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override { return true; }
    bool dependentWith(std::shared_ptr<MCTransition>) override { return false; }
    bool countsAgainstThreadExecutionDepth() override { return false; }
    void print() override;
};

#endif //MCMINI_MCTHREADENTERGOALCRITICALSECTION_H
