#ifndef MCMINI_MCTHREADEXITGOALCRITICALSECTION_H
#define MCMINI_MCTHREADEXITGOALCRITICALSECTION_H

#include "MCThreadTransition.h"

MCTransition* MCReadThreadExitGoalCriticalSection(const MCSharedTransition*, void*, MCState*);

struct MCThreadExitGoalCriticalSection : MCThreadTransition {
    inline explicit MCThreadExitGoalCriticalSection(std::shared_ptr<MCThread> thread) : MCThreadTransition(thread) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override { return true; }
    bool dependentWith(std::shared_ptr<MCTransition>) override { return false; }
    bool countsAgainstThreadExecutionDepth() override { return false; }
    void print() override;
};

#endif //MCMINI_MCTHREADEXITGOALCRITICALSECTION_H
