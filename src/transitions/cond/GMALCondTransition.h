#ifndef GMAL_GMALCONDTRANSITION_H
#define GMAL_GMALCONDTRANSITION_H

#include "GMALTransition.h"
#include "objects/GMALConditionVariable.h"

struct GMALCondTransition : public GMALTransition {
public:
    std::shared_ptr<GMALConditionVariable> conditionVariable;
    GMALCondTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALConditionVariable> conditionVariable) :
            GMALTransition(running), conditionVariable(conditionVariable) {}
};


#endif //GMAL_GMALCONDTRANSITION_H
