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
}