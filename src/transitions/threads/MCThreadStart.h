#ifndef MC_MCTHREADSTART_H
#define MC_MCTHREADSTART_H

#include "MCShared.h"
#include "MCThreadTransition.h"

MCTransition* MCReadThreadStart(const MCSharedTransition*, void*, MCState*);

struct MCThreadStart : public MCThreadTransition {
public:
    inline explicit MCThreadStart(std::shared_ptr<MCThread> thread) : MCThreadTransition(thread) {}

    std::shared_ptr<MCTransition> staticCopy() const override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) const override;
    void applyToState(MCState *) override;
    bool coenabledWith(const MCTransition*) const override;
    bool dependentWith(const MCTransition*) const override;
    bool countsAgainstThreadExecutionDepth() const override;
    void print() const override;
};

#endif //MC_MCTHREADSTART_H
