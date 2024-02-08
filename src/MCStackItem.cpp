#include "mcmini/MCStackItem.h"
#include <algorithm>
using namespace std;

void
MCStackItem::addBacktrackingThreadIfUnsearched(tid_t tid)
{
  bool containedInDoneSet = this->doneSet.count(tid) > 0;
  if (!containedInDoneSet) { this->backtrackSet.insert(tid); }
}

void
MCStackItem::markBacktrackThreadSearched(tid_t tid)
{
  this->doneSet.insert(tid);
  this->backtrackSet.erase(tid);
}

bool
MCStackItem::hasThreadsToBacktrackOn() const
{
  return !backtrackSet.empty();
}

bool
MCStackItem::isBacktrackingOnThread(tid_t tid) const
{
  return this->backtrackSet.count(tid) > 0;
}

bool
MCStackItem::threadIsInSleepSet(tid_t tid) const
{
  // If the thread runs a transition contained in the
  // sleep set, we know that it is the only such transition
  // in the sleep set. See the comment below
  for (const tid_t &t : this->sleepSet)
    if (t == tid) return true;
  return false;
}

tid_t
MCStackItem::popThreadToBacktrackOn()
{
  MC_ASSERT(this->hasThreadsToBacktrackOn());

  // Arbitrarily always pick the smallest thread
  // to provide a determinism (e.g.)
  tid_t backtrack_thread = *this->backtrackSet.begin();

  for (const tid_t tid : this->backtrackSet) {
    if (tid < backtrack_thread) { backtrack_thread = tid; }
  }

  this->markBacktrackThreadSearched(backtrack_thread);
  return backtrack_thread;
}

void
MCStackItem::markThreadsEnabledInState(
  const unordered_set<tid_t> &enabledThrds)
{
  for (const tid_t tid : enabledThrds)
    this->enabledThreads.insert(tid);
}

unordered_set<tid_t>
MCStackItem::getEnabledThreadsInState() const
{
  return this->enabledThreads;
}

unordered_set<tid_t>
MCStackItem::getSleepSet() const
{
  return this->sleepSet;
}

MCClockVector
MCStackItem::getClockVector() const
{
  return this->clockVector;
}

bool
MCStackItem::isRevertible() const
{
  return this->spawningTransitionCanRevertState;
}

void
MCStackItem::addThreadToSleepSet(tid_t tid)
{
  this->sleepSet.insert(tid);
}
