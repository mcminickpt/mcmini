#include "GMALStateStackItem.h"

void
GMALStateStackItem::addBacktrackingThreadIfUnsearched(tid_t tid)
{
    bool containedInDoneSet = this->doneSet.count(tid) > 0;
    if (!containedInDoneSet) {
        this->backtrackSet.insert(tid);
    }
}

void
GMALStateStackItem::markBacktrackThreadSearched(tid_t tid)
{
    this->doneSet.insert(tid);
    this->backtrackSet.erase(tid);
}

bool
GMALStateStackItem::hasThreadsToBacktrackOn() const
{
    return !backtrackSet.empty();
}

tid_t
GMALStateStackItem::popFirstThreadToBacktrackOn()
{
    GMAL_ASSERT(this->hasThreadsToBacktrackOn());

    tid_t randomThreadInBacktrackSet = *this->backtrackSet.begin();
    this->backtrackSet.erase(this->backtrackSet.begin());

    this->markBacktrackThreadSearched(randomThreadInBacktrackSet);
    return randomThreadInBacktrackSet;
}

void
GMALStateStackItem::markThreadsEnabledInState(const std::unordered_set<tid_t>& enabledThrds)
{
    for (const auto tid : enabledThrds)
        this->enabledThreads.insert(tid);
}

std::unordered_set<tid_t>
GMALStateStackItem::getEnabledThreadsInState()
{
    return this->enabledThreads;
}