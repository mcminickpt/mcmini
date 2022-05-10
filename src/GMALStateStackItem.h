#ifndef GMAL_GMALSTATESTACKITEM_H
#define GMAL_GMALSTATESTACKITEM_H

#include "GMALShared.h"
#include <unordered_set>
#include <utility>

struct GMALStateStackItem final {
private:
    std::unordered_set<tid_t> backtrackSet;
    std::unordered_set<tid_t> doneSet;
    std::unordered_set<tid_t> enabledThreads; /* A cache of threads that are enabled in this state*/
public:

    void addBacktrackingThreadIfUnsearched(tid_t);
    void markBacktrackThreadSearched(tid_t);

    void markThreadsEnabledInState(const std::unordered_set<tid_t>& enabledThrds);
    std::unordered_set<tid_t> getEnabledThreadsInState();

    bool hasThreadsToBacktrackOn() const;
    tid_t popFirstThreadToBacktrackOn();
};

#endif //GMAL_GMALSTATESTACKITEM_H
