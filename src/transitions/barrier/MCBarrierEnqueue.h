#ifndef MC_MCBARRIERENQUEUE_H
#define MC_MCBARRIERENQUEUE_H

#include "MCBarrierTransition.h"

MCTransition* MCReadBarrierEnqueue(const MCSharedTransition*, void*, MCState*);

struct MCBarrierEnqueue : public MCBarrierTransition {
public:
    MCBarrierEnqueue(std::shared_ptr<MCThread> thread, std::shared_ptr<MCBarrier> barrier)
            : MCBarrierTransition(thread, barrier) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCBARRIERENQUEUE_H
