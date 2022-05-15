#ifndef GMAL_GMALGLOBALVARIABLEREAD_H
#define GMAL_GMALGLOBALVARIABLEREAD_H

#include "GMALGlobalVariableTransition.h"

GMALTransition* GMALReadGlobalRead(const GMALSharedTransition*, void*, GMALState*);

struct GMALGlobalVariableRead : public GMALGlobalVariableTransition {
public:
    GMALGlobalVariableRead(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALGlobalVariable> global) :
            GMALGlobalVariableTransition(running, global) {}

    std::shared_ptr<GMALTransition> staticCopy() override;
    std::shared_ptr<GMALTransition> dynamicCopyInState(const GMALState*) override;
    void applyToState(GMALState *) override {}
    bool coenabledWith(std::shared_ptr<GMALTransition>) override;
    bool dependentWith(std::shared_ptr<GMALTransition>) override;
    void print() override;
};

#endif //GMAL_GMALGLOBALVARIABLEREAD_H
