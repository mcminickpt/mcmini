#include "MCStateStackItem.h"
#include <algorithm>
using namespace std;

void
MCStateStackItem::addBacktrackingThreadIfUnsearched(tid_t tid)
{
    if (!hasBacktrackedOnThread(tid))
        this->backtrackSet.insert(tid);
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
MCStateStackItem::hasBacktrackedOnThread(tid_t tid) const
{
    return this->doneSet.count(tid) > 0;
}

bool
MCStateStackItem::isBacktrackingOnThread(tid_t tid) const
{
    return this->backtrackSet.count(tid) > 0;
}


bool 
MCStateStackItem::isPersistentSetRestartId(tid_t tid) const
{
    return this->persistentSetRestartId == tid;
}

bool
MCStateStackItem::threadIsInSleepSet(tid_t tid) const
{
    // If the thread runs a transition contained in the
    // sleep set, we know that it is the only such transition
    // in the sleep set. See the comment below
    for (const tid_t &t: this->sleepSet)
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
MCStateStackItem::markThreadsEnabledInState(const unordered_set<tid_t>& enabledThrds)
{
    for (const tid_t tid : enabledThrds)
        this->enabledThreads.insert(tid);
}

unordered_set<tid_t> 
MCStateStackItem::getUnexecutedPersistentSetCandidates()
{
    unordered_set<tid_t> candidates;
    for (const tid_t &tid : this->enabledThreads)
        if (!this->hasBacktrackedOnThread(tid) && !this->threadIsInSleepSet(tid))
            candidates.insert(tid);
    return candidates;
}

unordered_set<tid_t>
MCStateStackItem::getEnabledThreadsInState()
{
    return this->enabledThreads;
}

unordered_set<tid_t>
MCStateStackItem::getSleepSet()
{
    return this->sleepSet;
}

MCOptional<tid_t> 
MCStateStackItem::getPersistentSetRestartId()
{
    return this->persistentSetRestartId;
}

void
MCStateStackItem::addThreadToSleepSet(tid_t tid)
{
    this->sleepSet.insert(tid);
}

void 
MCStateStackItem::markPersistentSetRestartId(tid_t tid)
{
    this->persistentSetRestartId = MCOptional<tid_t>::of(tid);
}

void 
MCStateStackItem::backtrackWithNewPersistentSetId()
{
    unordered_set<tid_t> candidates = getUnexecutedPersistentSetCandidates();
    if (!candidates.empty()) { 
        // Pick a candidate arbitrarily
        tid_t tid = *candidates.begin();
        markPersistentSetRestartId(tid);
        addBacktrackingThreadIfUnsearched(tid);
    } else { 
        // If there are no candidates, that means that all enabled
        // threads that need to be searched (i.e. that are not in 
        // the sleep set) have already been explored from this state 
        // and thus we don't have to do anything
        this->persistentSetRestartId = MCOptional<tid_t>::nil();
    }
}