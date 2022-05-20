#ifndef MC_MCTHREADSTART_H
#define MC_MCTHREADSTART_H

#include "MCShared.h"
#include "MCThreadTransition.h"

MCTransition* MCReadThreadStart(const MCSharedTransition*, void*, MCState*);

struct MCThreadStart : public MCThreadTransition {
public:
    inline explicit MCThreadStart(std::shared_ptr<MCThread> thread) : MCThreadTransition(thread) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool countsAgainstThreadExecutionDepth() override;
    void print() override;
};

#endif //MC_MCTHREADSTART_H
