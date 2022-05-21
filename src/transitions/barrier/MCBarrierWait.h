#ifndef MC_MCBARRIERWAIT_H
#define MC_MCBARRIERWAIT_H

#include "MCBarrierTransition.h"

MCTransition* MCReadBarrierWait(const MCSharedTransition*, void*, MCState*);

struct MCBarrierWait : public MCBarrierTransition {
public:
    MCBarrierWait(std::shared_ptr<MCThread> thread, std::shared_ptr<MCBarrier> barrier)
            : MCBarrierTransition(thread, barrier) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;
    void print() override;
};

#endif //MC_MCBARRIERWAIT_H
