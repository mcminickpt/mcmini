#ifndef MC_MCGLOBALVARIABLEWRITE_H
#define MC_MCGLOBALVARIABLEWRITE_H

#include "MCGlobalVariableTransition.h"

MCTransition* MCReadGlobalWrite(const MCSharedTransition*, void*, MCState*);

struct MCGlobalVariableWriteData {
    void *addr;
    void *newValue;

    MCGlobalVariableWriteData(void *addr, void *newValue) : addr(addr), newValue(newValue) {}
};

struct MCGlobalVariableWrite: public MCGlobalVariableTransition {
public:
    const void *newValue;
    MCGlobalVariableWrite(std::shared_ptr<MCThread> running, std::shared_ptr<MCGlobalVariable> global, void *newValue) :
            MCGlobalVariableTransition(running, global), newValue(newValue) {}

    std::shared_ptr<MCTransition> staticCopy() override;
    std::shared_ptr<MCTransition> dynamicCopyInState(const MCState*) override;
    void applyToState(MCState *) override {}
    bool coenabledWith(std::shared_ptr<MCTransition>) override;
    bool dependentWith(std::shared_ptr<MCTransition>) override;
    bool isRacingWith(std::shared_ptr<MCTransition>) override;
    void print() override;
};

#endif //MC_MCGLOBALVARIABLEWRITE_H
