#ifndef GMAL_GMALSTATESTACKITEM_H
#define GMAL_GMALSTATESTACKITEM_H

#include "GMALShared.h"
#include <set>

struct GMALStateStackItem final {
private:
    std::set<tid_t> backtrackSet;
    std::set<tid_t> doneSet;
public:
    void addBacktrackingThreadIfUnsearched(tid_t);
    void markBacktrackThreadSearched(tid_t);
};

#endif //GMAL_GMALSTATESTACKITEM_H
