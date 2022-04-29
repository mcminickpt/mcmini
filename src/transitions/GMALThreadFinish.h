#ifndef GMAL_GMALTHREADFINISH_H
#define GMAL_GMALTHREADFINISH_H

#include "GMALThreadTransition.h"

GMALTransition*
GMALReadThreadFinish(const GMALSharedTransition *shmTransition, void *shmData, GMALState *state);

struct GMALThreadFinish : public GMALThreadTransition {
public:
    inline GMALThreadFinish(std::shared_ptr<GMALThread> threadRunning) :
    GMALThreadTransition(threadRunning, threadRunning) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
    bool enabledInState(const GMALState *) override;
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool ensuresDeadlockIsImpossible() override;

    void print() override;
};

#endif //GMAL_GMALTHREADFINISH_H
