#ifndef MC_MCSEMENQUEUE_H
#define MC_MCSEMENQUEUE_H

#include "MCSemaphoreTransition.h"

MCTransition* MCReadSemEnqueue(const MCSharedTransition*, void*, MCState*);

struct MCSemEnqueue : public MCSemaphoreTransition {
public:
    MCSemEnqueue(std::shared_ptr<MCThread> running, std::shared_ptr<MCSemaphore> sem) :
            MCSemaphoreTransition(running, sem) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCSEMENQUEUE_H
