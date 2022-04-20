#include "GMALTransition.h"
#include "GMALTransitionFactory.h"

bool
GMALTransition::dependentTransitions(const std::shared_ptr<GMALTransition>& t1, const std::shared_ptr<GMALTransition>& t2)
{
    return GMALTransitionFactory::transitionsDependentCommon(t1, t2) || t1->dependentWith(t2) || t2->dependentWith(t1);
}

bool
GMALTransition::coenabledTransitions(const std::shared_ptr<GMALTransition>& t1, const std::shared_ptr<GMALTransition>& t2)
{
    return GMALTransitionFactory::transitionsCoenabledCommon(t1, t2) && t1->coenabledWith(t2) && t2->coenabledWith(t1);
}