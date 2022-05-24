#include "MCTransition.h"
#include "MCTransitionFactory.h"

bool
MCTransition::dependentTransitions(const std::shared_ptr<MCTransition>& t1, const std::shared_ptr<MCTransition>& t2)
{
    return MCTransitionFactory::transitionsDependentCommon(t1, t2) || t1->dependentWith(t2) || t2->dependentWith(t1);
}

bool
MCTransition::coenabledTransitions(const std::shared_ptr<MCTransition>& t1, const std::shared_ptr<MCTransition>& t2)
{
    return MCTransitionFactory::transitionsCoenabledCommon(t1, t2) && t1->coenabledWith(t2) && t2->coenabledWith(t1);
}

bool
MCTransition::transitionsInDataRace(const std::shared_ptr<MCTransition> &t1, const std::shared_ptr<MCTransition> &t2)
{
    return t1->isRacingWith(t2) || t2->isRacingWith(t1);
}