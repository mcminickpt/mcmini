#include "mcmini/MCTransition.h"
#include "mcmini/MCState.h"
#include "mcmini/transitions/threads/MCThreadDefs.h"

bool
MCTransition::dependentTransitions(const MCTransition &t1,
                                   const MCTransition &t2)
{
  return MCTransition::dependentTransitions(&t1, &t2);
}

bool
MCTransition::coenabledTransitions(const MCTransition &t1,
                                   const MCTransition &t2)
{
  return MCTransition::coenabledTransitions(&t1, &t2);
}

bool
MCTransition::transitionsInDataRace(const MCTransition &t1,
                                    const MCTransition &t2)
{
  return MCTransition::transitionsInDataRace(&t1, &t2);
}

bool
MCTransition::transitionEnabledInState(const MCState *state,
                                       const MCTransition &t1)
{
  return MCTransition::transitionEnabledInState(state, &t1);
}

bool
MCTransition::dependentTransitions(const MCTransition *t1,
                                   const MCTransition *t2)
{
  return MCTransition::transitionsDependentCommon(t1, t2) ||
         t1->dependentWith(t2) || t2->dependentWith(t1);
}

bool
MCTransition::coenabledTransitions(const MCTransition *t1,
                                   const MCTransition *t2)
{
  return MCTransition::transitionsCoenabledCommon(t1, t2) &&
         t1->coenabledWith(t2) && t2->coenabledWith(t1);
}

bool
MCTransition::transitionsInDataRace(const MCTransition *t1,
                                    const MCTransition *t2)
{
  return t1->isRacingWith(t2) || t2->isRacingWith(t1);
}

bool
MCTransition::transitionEnabledInState(const MCState *state,
                                       const MCTransition *t1)
{
  // Is the thread enabled?
  return t1->threadIsEnabled() && t1->enabledInState(state);
}

bool
MCTransition::transitionsCoenabledCommon(const MCTransition *t1,
                                         const MCTransition *t2)
{
  if (t1->getThreadId() == t2->getThreadId()) return false;

  {
    const MCThreadCreate *maybeThreadCreate_t1 =
      dynamic_cast<const MCThreadCreate *>(t1);
    if (maybeThreadCreate_t1 != nullptr)
      return !maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());

    const MCThreadCreate *maybeThreadCreate_t2 =
      dynamic_cast<const MCThreadCreate *>(t2);
    if (maybeThreadCreate_t2)
      return !maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
  }

  {
    const MCThreadJoin *maybeThreadJoin_t1 =
      dynamic_cast<const MCThreadJoin *>(t1);
    if (maybeThreadJoin_t1)
      return !maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());

    const MCThreadJoin *maybeThreadJoin_t2 =
      dynamic_cast<const MCThreadJoin *>(t2);
    if (maybeThreadJoin_t2)
      return !maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
  }

  return true;
}

bool
MCTransition::transitionsDependentCommon(const MCTransition *t1,
                                         const MCTransition *t2)
{
  if (t1->getThreadId() == t2->getThreadId()) return true;

  {
    const MCThreadCreate *maybeThreadCreate_t1 =
      dynamic_cast<const MCThreadCreate *>(t1);
    if (maybeThreadCreate_t1 != nullptr)
      return maybeThreadCreate_t1->doesCreateThread(t2->getThreadId());

    const MCThreadCreate *maybeThreadCreate_t2 =
      dynamic_cast<const MCThreadCreate *>(t2);
    if (maybeThreadCreate_t2)
      return maybeThreadCreate_t2->doesCreateThread(t1->getThreadId());
  }

  {
    const MCThreadJoin *maybeThreadJoin_t1 =
      dynamic_cast<const MCThreadJoin *>(t1);
    if (maybeThreadJoin_t1)
      return maybeThreadJoin_t1->joinsOnThread(t2->getThreadId());

    const MCThreadJoin *maybeThreadJoin_t2 =
      dynamic_cast<const MCThreadJoin *>(t2);
    if (maybeThreadJoin_t2)
      return maybeThreadJoin_t2->joinsOnThread(t1->getThreadId());
  }

  return false;
}
