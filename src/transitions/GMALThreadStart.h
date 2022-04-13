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

    bool
    dependentWith(const GMALTransition &other) override
    {
        return true;
    }

    bool
    conenabledWith(const GMALTransition &other) override
    {
        return true;
    }
};

#endif //GMAL_GMALTHREADSTART_H
