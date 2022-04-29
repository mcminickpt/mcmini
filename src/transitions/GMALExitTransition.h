#ifndef GMAL_GMALEXITTRANSITION_H
#define GMAL_GMALEXITTRANSITION_H

#include "GMALTransition.h"

GMALTransition* GMALReadExitTransition(const GMALSharedTransition*, void*, GMALState*);

struct GMALExitTransition : public GMALTransition {
private:
    const int exitCode;
public:
    GMALExitTransition(std::shared_ptr<GMALThread> thread, int exitCode)
    : GMALTransition(thread), exitCode(exitCode) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;

    void applyToState(GMALState *) override {}
    void unapplyToState(GMALState *) override {}
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    bool enabledInState(const GMALState *) override;
    bool ensuresDeadlockIsImpossible() override;

    void print() override;
};

#endif //GMAL_GMALEXITTRANSITION_H
