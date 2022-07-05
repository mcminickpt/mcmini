#ifndef MC_MCSTATESTACKITEM_H
#define MC_MCSTATESTACKITEM_H

#include "MCShared.h"
#include "MCTransition.h"
#include <unordered_set>
#include <utility>

struct MCStateStackItem final {
private:
    std::unordered_set<tid_t> backtrackSet;
    std::unordered_set<tid_t> doneSet;
    std::unordered_set<std::shared_ptr<MCTransition>> sleepSet;

    /* A cache of threads that are enabled in this state */
    std::unordered_set<tid_t> enabledThreads;
public:

    void addBacktrackingThreadIfUnsearched(tid_t);
    void markBacktrackThreadSearched(tid_t);

    void markThreadsEnabledInState(const std::unordered_set<tid_t>&);
    std::unordered_set<tid_t> getEnabledThreadsInState();

    void addTransitionToSleepSet(std::shared_ptr<MCTransition>);

    std::unordered_set<std::shared_ptr<MCTransition>>
    newFilteredSleepSet(std::shared_ptr<MCTransition>);

    bool hasThreadsToBacktrackOn() const;
    bool isBacktrackingOnThread(tid_t) const;
    tid_t popFirstThreadToBacktrackOn();
};

#endif //MC_MCSTATESTACKITEM_H
