#ifndef GMAL_GMALGLOBALVARIABLETRANSITION_H
#define GMAL_GMALGLOBALVARIABLETRANSITION_H

#include "GMALTransition.h"
#include "objects/GMALGlobalVariable.h"

struct GMALGlobalVariableTransition : public GMALTransition {
public:
    std::shared_ptr<GMALGlobalVariable> global;
    GMALGlobalVariableTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALGlobalVariable> global) :
            GMALTransition(running), global(global) {}
};

#endif //GMAL_GMALGLOBALVARIABLETRANSITION_H
