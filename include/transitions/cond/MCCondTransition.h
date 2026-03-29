#ifndef MC_MCCONDTRANSITION_H
#define MC_MCCONDTRANSITION_H

#include "MCTransition.h"
#include "objects/MCConditionVariable.h"

struct MCCondTransition : public MCTransition {
 public:
  std::shared_ptr<MCConditionVariable> conditionVariable;
  bool hadWaiters;
  MCCondTransition(std::shared_ptr<MCThread> running,
                   std::shared_ptr<MCConditionVariable> conditionVariable)
      : MCTransition(running),
        conditionVariable(conditionVariable),
        hadWaiters(conditionVariable->hasWaiters()) {}

  std::unordered_set<objid_t> getObjectsAccessedByTransition() const override {
    std::unordered_set<objid_t> ids = { conditionVariable->getObjectId() };
    if (conditionVariable->mutex != nullptr)
      ids.insert(conditionVariable->mutex->getObjectId());
    return ids;
  }
};

#endif  // MC_MCCONDTRANSITION_H
