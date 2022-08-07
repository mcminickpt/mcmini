#include "mcmini/MCStateStackItem.h"
#include <algorithm>
using namespace std;

void
MCStateStackItem::addBacktrackingThreadIfUnsearched(tid_t tid)
{
  bool containedInDoneSet = this->doneSet.count(tid) > 0;
  if (!containedInDoneSet) { this->backtrackSet.insert(tid); }
}

void
MCStateStackItem::markBacktrackThreadSearched(tid_t tid)
{
  this->doneSet.insert(tid);
  this->backtrackSet.erase(tid);
}

bool
MCStateStackItem::hasThreadsToBacktrackOn() const
{
  return !backtrackSet.empty();
}

bool
MCStateStackItem::isBacktrackingOnThread(tid_t tid) const
{
  return this->backtrackSet.count(tid) > 0;
}

bool
MCStateStackItem::threadIsInSleepSet(tid_t tid) const
{
  // If the thread runs a transition contained in the
  // sleep set, we know that it is the only such transition
  // in the sleep set. See the comment below
  for (const tid_t &t : this->sleepSet)
    if (t == tid) return true;
  return false;
}

tid_t
MCStateStackItem::popThreadToBacktrackOn()
{
  MC_ASSERT(this->hasThreadsToBacktrackOn());
  tid_t randomThreadInBacktrackSet = *this->backtrackSet.begin();
  this->markBacktrackThreadSearched(randomThreadInBacktrackSet);
  return randomThreadInBacktrackSet;
}

void
MCStateStackItem::markThreadsEnabledInState(
  const unordered_set<tid_t> &enabledThrds)
{
  for (const tid_t tid : enabledThrds)
    this->enabledThreads.insert(tid);
}

unordered_set<tid_t>
MCStateStackItem::getEnabledThreadsInState() const
{
  return this->enabledThreads;
}

unordered_set<tid_t>
MCStateStackItem::getSleepSet() const
{
  return this->sleepSet;
}

MCClockVector
MCStateStackItem::getClockVector() const
{
  return this->clockVector;
}

bool
MCStateStackItem::isRevertible() const
{
  return this->spawningTransitionCanRevertState;
}

void
MCStateStackItem::addThreadToSleepSet(tid_t tid)
{
  this->sleepSet.insert(tid);
}