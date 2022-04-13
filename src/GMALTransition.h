#ifndef GMAL_GMALTRANSITION_H
#define GMAL_GMALTRANSITION_H

struct GMALState;

#include "GMALShared.h"
#include "GMALState.h"
#include "GMALThread.h"
#include <memory>
#include <utility>

struct GMALTransition {
protected:
    std::shared_ptr<GMALThread> thread;
public:
    GMALTransition(std::shared_ptr<GMALThread> thread) : thread(thread) {}

    GMALTransition(const GMALTransition&) = default;
    GMALTransition &operator=(const GMALTransition&) = default;

    static GMALTransition *staticCopy() { return nullptr; }

    virtual void applyToState(GMALState &state) {}
    virtual void unapplyToState(GMALState &state) {}
    virtual bool enabledInState(const GMALState &state) { return true; }
    virtual bool conenabledWith(const GMALTransition &other) { return true; }
    virtual bool dependentWith(const GMALTransition &other) { return true; }

    tid_t getThreadId() { return thread->tid; }
};

#endif //GMAL_GMALTRANSITION_H
