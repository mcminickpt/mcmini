#ifndef GMAL_GMALSTATESTACKITEM_H
#define GMAL_GMALSTATESTACKITEM_H

#include "GMALShared.h"
#include <unordered_set>

struct GMALStateStackItem final {
private:
    std::unordered_set<tid_t> backtrackSet;
    std::unordered_set<tid_t> doneSet;
public:
    void addBacktrackingThreadIfUnsearched(tid_t);
    void markBacktrackThreadSearched(tid_t);

    bool hasThreadsToBacktrackOn() const;
    tid_t popFirstThreadToBacktrackOn();
};

#endif //GMAL_GMALSTATESTACKITEM_H
