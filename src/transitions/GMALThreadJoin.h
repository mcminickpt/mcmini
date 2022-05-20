#ifndef MC_MCTHREADJOIN_H
#define MC_MCTHREADJOIN_H

#include "MCShared.h"
#include "MCThreadTransition.h"

MCTransition* MCReadThreadJoin(const MCSharedTransition*, void*, MCState*);

struct MCThreadJoin : public MCThreadTransition {
public:
    inline MCThreadJoin(std::shared_ptr<MCThread> threadRunning, std::shared_ptr<MCThread> joinedOn) :
            MCThreadTransition(threadRunning, joinedOn) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override;
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool enabledInState(const MCState *) override;

    bool joinsOnThread(tid_t) const;
    bool joinsOnThread(const std::shared_ptr<MCThread>&) const;

    void print() override;
};

#endif //MC_MCTHREADJOIN_H
