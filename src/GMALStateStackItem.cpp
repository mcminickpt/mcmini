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
    tid_t randomThreadInBacktrackSet = *this->backtrackSet.end();
    this->markBacktrackThreadSearched(randomThreadInBacktrackSet);
    return randomThreadInBacktrackSet;
}