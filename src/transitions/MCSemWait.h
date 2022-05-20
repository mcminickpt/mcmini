#ifndef MC_MCSEMWAIT_H
#define MC_MCSEMWAIT_H

#include "MCSemaphoreTransition.h"

MCTransition* MCReadSemWait(const MCSharedTransition*, void*, MCState*);

struct MCSemWait : public MCSemaphoreTransition {
public:
    MCSemWait(std::shared_ptr<MCThread> running, std::shared_ptr<MCSemaphore> sem) :
            MCSemaphoreTransition(running, sem) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;
    void print() override;
};


#endif //MC_MCSEMWAIT_H
