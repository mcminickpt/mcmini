#ifndef MC_MCBARRIERINIT_H
#define MC_MCBARRIERINIT_H

#include "MCBarrierTransition.h"

MCTransition* MCReadBarrierInit(const MCSharedTransition*, void*, MCState*);

struct MCBarrierInit : public MCBarrierTransition {
public:
    MCBarrierInit(std::shared_ptr<MCThread> thread, std::shared_ptr<MCBarrier> barrier)
    : MCBarrierTransition(thread, barrier) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCBARRIERINIT_H
