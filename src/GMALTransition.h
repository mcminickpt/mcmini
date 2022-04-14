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

    static bool dependentTransitions(const std::shared_ptr<GMALTransition>&, const std::shared_ptr<GMALTransition>&);
    static bool coenabledTransitions(const std::shared_ptr<GMALTransition>&, const std::shared_ptr<GMALTransition>&);

    virtual void applyToState(GMALState *) {}
    virtual void unapplyToState(const GMALState *) {}
    virtual bool enabledInState(const GMALState *) { return true; }
    virtual bool coenabledWith(std::shared_ptr<GMALTransition>) { return true; }
    virtual bool dependentWith(std::shared_ptr<GMALTransition>) { return true; }

    tid_t getThreadId() { return thread->tid; }
};

#endif //GMAL_GMALTRANSITION_H
