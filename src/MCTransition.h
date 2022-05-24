#ifndef MC_MCTRANSITION_H
#define MC_MCTRANSITION_H

struct MCState;

#include "MCShared.h"
#include "MCState.h"
#include "objects/MCThread.h"
#include <memory>
#include <utility>

struct MCTransition {
protected:
    std::shared_ptr<MCThread> thread;
public:
    MCTransition(std::shared_ptr<MCThread> thread) : thread(thread) {}
    MCTransition(const MCTransition&) = default;
    MCTransition &operator=(const MCTransition&) = default;

    static bool dependentTransitions(const std::shared_ptr<MCTransition>&, const std::shared_ptr<MCTransition>&);
    static bool coenabledTransitions(const std::shared_ptr<MCTransition>&, const std::shared_ptr<MCTransition>&);

    virtual std::shared_ptr<MCTransition> staticCopy() = 0;
    virtual std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) = 0;
    virtual void applyToState(MCState *) = 0;
    virtual bool enabledInState(const MCState *) { return thread->enabled(); }
    virtual bool coenabledWith(std::shared_ptr<MCTransition>) { return true; }
    virtual bool dependentWith(std::shared_ptr<MCTransition>) { return true; }
    virtual bool canRaceWith(std::shared_ptr<MCTransition>) { return false; }

    /**
     *
     * @return
     */
    virtual bool ensuresDeadlockIsImpossible() { return false; }

    /**
     * Determines whether or not this transition should be considered
     * when determining the number of transitions run
     * @return
     */
    virtual bool countsAgainstThreadExecutionDepth() { return true; }

    // Printing
    virtual void print() {}

    inline tid_t getThreadId() const { return thread->tid; }
};

#endif //MC_MCTRANSITION_H
