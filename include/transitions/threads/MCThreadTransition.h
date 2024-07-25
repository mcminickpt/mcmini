#ifndef MC_MCTHREADTRANSITION_H
#define MC_MCTHREADTRANSITION_H

#include "mcmini/MCTransition.h"

struct MCThreadTransition : public MCTransition {
protected:
  std::shared_ptr<MCThread> target;

public:
  MCThreadTransition(std::shared_ptr<MCThread> running,
                     std::shared_ptr<MCThread> target)
    : MCTransition(running), target(target)
  {
  }
  MCThreadTransition(std::shared_ptr<MCThread> runningThread)
    : MCThreadTransition(runningThread, runningThread)
  {
  }
};

#endif // MC_MCTHREADTRANSITION_H
