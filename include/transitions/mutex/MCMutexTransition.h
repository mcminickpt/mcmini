#ifndef MC_MCMUTEXTRANSITION_H
#define MC_MCMUTEXTRANSITION_H

#include "MCTransition.h"
#include "objects/MCMutex.h"

struct MCMutexTransition : public MCTransition {
public:

  std::shared_ptr<MCMutex> mutex;
  MCMutexTransition(std::shared_ptr<MCThread> running,
                    std::shared_ptr<MCMutex> mutex)
    : MCTransition(running), mutex(mutex)
  {}

  std::unordered_set<objid_t> getObjectsAccessedByTransition() const override {
    return { mutex->getObjectId() };
  }
};

#endif // MC_MCMUTEXTRANSITION_H
