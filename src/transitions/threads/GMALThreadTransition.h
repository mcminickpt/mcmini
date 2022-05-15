#ifndef GMAL_GMALTHREADTRANSITION_H
#define GMAL_GMALTHREADTRANSITION_H

#include "GMALTransition.h"

struct GMALThreadTransition : public GMALTransition {
protected:
    std::shared_ptr<GMALThread> target;
public:
    GMALThreadTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALThread> target) : GMALTransition(running), target(target) {}
    GMALThreadTransition(std::shared_ptr<GMALThread> runningThread) : GMALThreadTransition(runningThread, runningThread) {}
};


#endif //GMAL_GMALTHREADTRANSITION_H
