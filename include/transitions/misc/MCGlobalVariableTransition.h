#ifndef MC_MCGLOBALVARIABLETRANSITION_H
#define MC_MCGLOBALVARIABLETRANSITION_H

#include "MCTransition.h"
#include "objects/MCGlobalVariable.h"

struct MCGlobalVariableTransition : public MCTransition {
public:

  std::shared_ptr<MCGlobalVariable> global;
  MCGlobalVariableTransition(std::shared_ptr<MCThread> running,
                             std::shared_ptr<MCGlobalVariable> global)
    : MCTransition(running), global(global)
  {}

  std::unordered_set<objid_t> getObjectIds() const override {
    return { global->getObjectId() };
  }
};

#endif // MC_MCGLOBALVARIABLETRANSITION_H
