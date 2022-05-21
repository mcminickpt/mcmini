#ifndef MC_MCSEMINIT_H
#define MC_MCSEMINIT_H

#include "MCSemaphoreTransition.h"

MCTransition* MCReadSemInit(const MCSharedTransition*, void*, MCState*);

struct MCSemInit : public MCSemaphoreTransition {
public:
    MCSemInit(std::shared_ptr<MCThread> running, std::shared_ptr<MCSemaphore> sem) :
            MCSemaphoreTransition(running, sem) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCSEMINIT_H
