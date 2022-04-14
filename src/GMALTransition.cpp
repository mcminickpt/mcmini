//
// Created by parallels on 4/10/22.
//

#include "GMALTransition.h"

bool
GMALTransition::dependentTransitions(const std::shared_ptr<GMALTransition>& t1, const std::shared_ptr<GMALTransition>& t2)
{
    return t1->dependentWith(t2) || t2->dependentWith(t1);
}

bool
GMALTransition::coenabledTransitions(const std::shared_ptr<GMALTransition>& t1, const std::shared_ptr<GMALTransition>& t2)
{
    return t1->coenabledWith(t2) || t2->coenabledWith(t1);
}