#ifndef MC_MCTHREADCREATE_H
#define MC_MCTHREADCREATE_H

#include "MCShared.h"
#include "MCThreadTransition.h"

MCTransition* MCReadThreadCreate(const MCSharedTransition*, void*, MCState*);

struct MCThreadCreate : public MCThreadTransition {
public:
    inline MCThreadCreate(std::shared_ptr<MCThread> threadRunning, std::shared_ptr<MCThread> threadCreated) :
    MCThreadTransition(threadRunning, threadCreated) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;

    bool doesCreateThread(tid_t) const;

    void print() override;
};

#endif //MC_MCTHREADCREATE_H
