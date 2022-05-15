#ifndef GMAL_GMALGLOBALVARIABLEWRITE_H
#define GMAL_GMALGLOBALVARIABLEWRITE_H

#include "GMALGlobalVariableTransition.h"

GMALTransition* GMALReadGlobalWrite(const GMALSharedTransition*, void*, GMALState*);

struct GMALGlobalVariableWriteData {
    void *addr;
    void *newValue;

    GMALGlobalVariableWriteData(void *addr, void *newValue) : addr(addr), newValue(newValue) {}
};

struct GMALGlobalVariableWrite: public GMALGlobalVariableTransition {
public:
    const void *newValue;
    GMALGlobalVariableWrite(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALGlobalVariable> global, void *newValue) :
            GMALGlobalVariableTransition(running, global), newValue(newValue) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override {}
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALGLOBALVARIABLEWRITE_H
