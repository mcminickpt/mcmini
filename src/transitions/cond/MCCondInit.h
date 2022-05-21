#ifndef MC_MCCONDINIT_H
#define MC_MCCONDINIT_H

#include "MCCondTransition.h"

MCTransition* MCReadCondInit(const MCSharedTransition*, void*, MCState*);

struct MCCondInit : public MCCondTransition {
public:
    MCCondInit(std::shared_ptr<MCThread> thread, std::shared_ptr<MCConditionVariable> cond)
            : MCCondTransition(thread, cond) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCCONDINIT_H
