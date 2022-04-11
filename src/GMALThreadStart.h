#ifndef GMAL_GMALTHREADSTART_H
#define GMAL_GMALTHREADSTART_H

#include "GMALShared.h"
#include "GMALTransition.h"

struct GMALThreadTransition : public GMALTransition {
protected:
    GMALRef<GMALThread> target;
public:
    GMALThreadTransition(GMALRef<GMALThread> running, GMALRef<GMALThread> target) : GMALTransition(running), target(target) {}
    GMALThreadTransition(GMALRef<GMALThread> runningThread) : GMALThreadTransition(runningThread, runningThread) {}
};

struct GMALThreadStart : public GMALThreadTransition {
public:
    inline explicit GMALThreadStart(GMALRef<GMALThread> thread) : GMALThreadTransition(std::move(thread)) {}

    bool
    dependentWith(const GMALTransition &other) override
    {


        return true;
    }

    void
    applyToState(GMALState &state) override
    {

    }

    void
    unapplyToState(GMALState &state) override
    {

    }

};

#endif //GMAL_GMALTHREADSTART_H
