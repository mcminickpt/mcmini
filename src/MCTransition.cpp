#include "MCTransition.h"
#include "MCTransitionFactory.h"

bool
MCTransition::dependentTransitions(const MCTransition &t1, const MCTransition &t2)
{
    return MCTransition::dependentTransitions(&t1, &t2);
}

bool
MCTransition::coenabledTransitions(const MCTransition &t1, const MCTransition &t2)
{
   return MCTransition::coenabledTransitions(&t1, &t2);
}

bool
MCTransition::transitionsInDataRace(const MCTransition &t1, const MCTransition &t2)
{
    return MCTransition::transitionsInDataRace(&t1, &t2);
}

bool
MCTransition::dependentTransitions(const MCTransition *t1, const MCTransition *t2)
{
    return MCTransitionFactory::transitionsDependentCommon(t1, t2) || t1->dependentWith(t2) || t2->dependentWith(t1);
}

bool
MCTransition::coenabledTransitions(const MCTransition *t1, const MCTransition *t2)
{
    return MCTransitionFactory::transitionsCoenabledCommon(t1, t2) && t1->coenabledWith(t2) && t2->coenabledWith(t1);
}

bool
MCTransition::transitionsInDataRace(const MCTransition *t1, const MCTransition *t2)
{
    return t1->isRacingWith(t2) || t2->isRacingWith(t1);
}