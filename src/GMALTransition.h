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

    static bool dependentTransitions(const std::shared_ptr<GMALTransition>&, const std::shared_ptr<GMALTransition>&);
    static bool coenabledTransitions(const std::shared_ptr<GMALTransition>&, const std::shared_ptr<GMALTransition>&);

    virtual std::shared_ptr<GMALTransition> staticCopy() = 0;
    virtual std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) = 0;
    virtual void applyToState(GMALState *) = 0;
    virtual void unapplyToState(GMALState *) = 0;
    virtual bool enabledInState(const GMALState *) { return thread->enabled(); }
    virtual bool coenabledWith(std::shared_ptr<GMALTransition>) { return true; }
    virtual bool dependentWith(std::shared_ptr<GMALTransition>) { return true; }

    virtual void print() = 0;
    inline tid_t getThreadId() const { return thread->tid; }
};

#endif //GMAL_GMALTRANSITION_H
