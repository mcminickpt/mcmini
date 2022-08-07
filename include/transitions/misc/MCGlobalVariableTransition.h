#ifndef MC_MCGLOBALVARIABLETRANSITION_H
#define MC_MCGLOBALVARIABLETRANSITION_H

#include "mcmini/MCTransition.h"
#include "mcmini/objects/MCGlobalVariable.h"

struct MCGlobalVariableTransition : public MCTransition {
public:

  std::shared_ptr<MCGlobalVariable> global;
  MCGlobalVariableTransition(std::shared_ptr<MCThread> running,
                             std::shared_ptr<MCGlobalVariable> global)
    : MCTransition(running), global(global)
  {}
};

#endif // MC_MCGLOBALVARIABLETRANSITION_H
