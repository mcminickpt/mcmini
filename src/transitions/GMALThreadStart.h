#ifndef GMAL_GMALTHREADSTART_H
#define GMAL_GMALTHREADSTART_H

#include "GMALShared.h"
#include "GMALTransition.h"

GMALTransition* GMALReadThreadStart(void *, const GMALState&);

struct GMALThreadTransition : public GMALTransition {
protected:
    std::shared_ptr<GMALThread> target;
public:
    GMALThreadTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALThread> target) : GMALTransition(running), target(target) {}
    GMALThreadTransition(std::shared_ptr<GMALThread> runningThread) : GMALThreadTransition(runningThread, runningThread) {}
};

struct GMALThreadStart : public GMALThreadTransition {
public:
    inline explicit GMALThreadStart(std::shared_ptr<GMALThread> thread) : GMALThreadTransition(thread) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override;
    void unapplyToState(GMALState *) override;
};

#endif //GMAL_GMALTHREADSTART_H
