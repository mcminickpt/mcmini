#ifndef MC_MCGLOBALVARIABLEREAD_H
#define MC_MCGLOBALVARIABLEREAD_H

#include "MCGlobalVariableTransition.h"

MCTransition* MCReadGlobalRead(const MCSharedTransition*, void*, MCState*);

struct MCGlobalVariableRead : public MCGlobalVariableTransition {
public:
    MCGlobalVariableRead(std::shared_ptr<MCThread> running, std::shared_ptr<MCGlobalVariable> global) :
            MCGlobalVariableTransition(running, global) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override {}
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool isRacingWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCGLOBALVARIABLEREAD_H
