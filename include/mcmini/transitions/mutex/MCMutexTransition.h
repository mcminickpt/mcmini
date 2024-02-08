#ifndef MC_MCMUTEXTRANSITION_H
#define MC_MCMUTEXTRANSITION_H

#include "mcmini/MCTransition.h"
#include "mcmini/objects/MCMutex.h"

struct MCMutexTransition : public MCTransition {
public:

  std::shared_ptr<MCMutex> mutex;
  MCMutexTransition(std::shared_ptr<MCThread> running,
                    std::shared_ptr<MCMutex> mutex)
    : MCTransition(running), mutex(mutex)
  {}
};

#endif // MC_MCMUTEXTRANSITION_H
