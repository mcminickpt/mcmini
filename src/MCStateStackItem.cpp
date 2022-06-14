#include "MCStateStackItem.h"

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

tid_t
MCStateStackItem::popFirstThreadToBacktrackOn()
{
    MC_ASSERT(this->hasThreadsToBacktrackOn());

    tid_t randomThreadInBacktrackSet = *this->backtrackSet.begin();
    this->backtrackSet.erase(this->backtrackSet.begin());

    this->markBacktrackThreadSearched(randomThreadInBacktrackSet);
    return randomThreadInBacktrackSet;
}

void
MCStateStackItem::markThreadsEnabledInState(const std::unordered_set<tid_t>& enabledThrds)
{
    for (const tid_t tid : enabledThrds)
        this->enabledThreads.insert(tid);
}

std::unordered_set<tid_t>
MCStateStackItem::getEnabledThreadsInState()
{
    return this->enabledThreads;
}