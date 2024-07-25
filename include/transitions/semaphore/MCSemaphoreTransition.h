#ifndef MC_MCSEMAPHORETRANSITION_H
#define MC_MCSEMAPHORETRANSITION_H

#include "mcmini/MCTransition.h"
#include "mcmini/objects/MCSemaphore.h"

struct MCSemaphoreTransition : public MCTransition {
public:
  std::shared_ptr<MCSemaphore> sem;
  MCSemaphoreTransition(std::shared_ptr<MCThread> running,
                        std::shared_ptr<MCSemaphore> sem)
    : MCTransition(running), sem(sem)
  {
  }
};

#endif // MC_MCSEMAPHORETRANSITION_H
