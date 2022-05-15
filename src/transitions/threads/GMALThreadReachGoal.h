#ifndef GMAL_GMALTHREADREACHGOAL_H
#define GMAL_GMALTHREADREACHGOAL_H

#include "GMALThreadTransition.h"

GMALTransition* GMALReadThreadReachGoal(const GMALSharedTransition*, void*, GMALState*);

struct GMALThreadReachGoal : GMALThreadTransition {
    inline explicit GMALThreadReachGoal(std::shared_ptr<GMALThread> thread) : GMALThreadTransition(thread) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool countsAgainstThreadExecutionDepth() override;
    void print() override;
};

#endif //GMAL_GMALTHREADREACHGOAL_H
