#ifndef GMAL_GMALTRANSITION_H
#define GMAL_GMALTRANSITION_H

struct GMALState;

#include "GMALShared.h"
#include "GMALState.h"
#include "GMALRef.h"
#include "GMALThread.h"
#include <utility>

struct GMALTransition {
protected:
    GMALRef<GMALThread> thread;
public:
    GMALTransition(GMALRef<GMALThread> thread) : thread(std::move(thread)) {}

    virtual void applyToState(GMALState &state) {}
    virtual void unapplyToState(GMALState &state) {}
    virtual bool enabledInState(const GMALState &state) { return true; }
    virtual bool conenabledWith(const GMALTransition &other) { return true; }
    virtual bool dependentWith(const GMALTransition &other) { return true; }
};

#endif //GMAL_GMALTRANSITION_H
