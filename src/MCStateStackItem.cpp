#include "MCStateStackItem.h"
#include <algorithm>
using namespace std;

void
MCStateStackItem::addBacktrackingThreadIfUnsearched(tid_t tid)
{
    bool containedInDoneSet = this->doneSet.count(tid) > 0;
    if (!containedInDoneSet) {
        this->backtrackSet.insert(tid);
    }
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
    for (const shared_ptr<MCTransition>& t : this->sleepSet)
        if (t->getThreadId() == tid) return true;
    return false;
}

tid_t
MCStateStackItem::popFirstThreadToBacktrackOn()
{
    MC_ASSERT(this->hasThreadsToBacktrackOn());

    tid_t randomThreadInBacktrackSet = *this->backtrackSet.begin();
    this->markBacktrackThreadSearched(randomThreadInBacktrackSet);
    return randomThreadInBacktrackSet;
}

void
MCStateStackItem::markThreadsEnabledInState(const unordered_set<tid_t>& enabledThrds)
{
    for (const tid_t tid : enabledThrds)
        this->enabledThreads.insert(tid);
}

unordered_set<tid_t>
MCStateStackItem::getEnabledThreadsInState()
{
    return this->enabledThreads;
}

void
MCStateStackItem::addTransitionToSleepSet(shared_ptr<MCTransition> transition)
{
    this->sleepSet.insert(transition);
}

unordered_set<shared_ptr<MCTransition>>
MCStateStackItem::newFilteredSleepSet(shared_ptr<MCTransition> transition)
{
    unordered_set<shared_ptr<MCTransition>> newSleepSet;
    for (const shared_ptr<MCTransition> &t : this->sleepSet) {
        if (!MCTransition::dependentTransitions(t, transition))
            newSleepSet.insert(t);
    }
    return newSleepSet;
}