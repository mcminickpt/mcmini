#ifndef MC_MCCONDWAIT_H
#define MC_MCCONDWAIT_H

#include "MCCondTransition.h"

MCTransition* MCReadCondWait(const MCSharedTransition*, void*, MCState*);

/**
 * Attempts to re-acquire the mutex/exit the thread queue
 */
struct MCCondWait : public MCCondTransition {
public:
    MCCondWait(std::shared_ptr<MCThread> running, std::shared_ptr<MCConditionVariable> cond) :
            MCCondTransition(running, cond) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;
    void print() override;
};

#endif //MC_MCCONDWAIT_H
