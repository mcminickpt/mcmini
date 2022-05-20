#ifndef MC_MCTHREADFINISH_H
#define MC_MCTHREADFINISH_H

#include "MCThreadTransition.h"

MCTransition*
MCReadThreadFinish(const MCSharedTransition *shmTransition, void *shmData, MCState *state);

struct MCThreadFinish : public MCThreadTransition {
public:
    inline explicit MCThreadFinish(std::shared_ptr<MCThread> threadRunning) :
    MCThreadTransition(threadRunning, threadRunning) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool enabledInState(const MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool ensuresDeadlockIsImpossible() override;
    bool countsAgainstThreadExecutionDepth() override;
    void print() override;
};

#endif //MC_MCTHREADFINISH_H
