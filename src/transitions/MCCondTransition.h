#ifndef MC_MCCONDTRANSITION_H
#define MC_MCCONDTRANSITION_H

#include "MCTransition.h"
#include "objects/MCConditionVariable.h"

struct MCCondTransition : public MCTransition {
public:
    std::shared_ptr<MCConditionVariable> conditionVariable;
    MCCondTransition(std::shared_ptr<MCThread> running, std::shared_ptr<MCConditionVariable> conditionVariable) :
            MCTransition(running), conditionVariable(conditionVariable) {}
};


#endif //MC_MCCONDTRANSITION_H
