#ifndef MC_MCEXITTRANSITION_H
#define MC_MCEXITTRANSITION_H

#include "MCTransition.h"

MCTransition* MCReadExitTransition(const MCSharedTransition*, void*, MCState*);

struct MCExitTransition : public MCTransition {
private:
    const int exitCode;
public:
    MCExitTransition(std::shared_ptr<MCThread> thread, int exitCode)
    : MCTransition(thread), exitCode(exitCode) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;

    void applyToState(MCState *) override {}
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;
    bool ensuresDeadlockIsImpossible() override;
    bool countsAgainstThreadExecutionDepth() override;
    void print() override;
};

#endif //MC_MCEXITTRANSITION_H
