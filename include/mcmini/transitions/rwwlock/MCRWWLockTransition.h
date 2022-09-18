#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKTRANSITION_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKTRANSITION_HPP

#include "mcmini/MCTransition.h"
#include "mcmini/objects/MCRWWLock.h"

struct MCRWWLockTransition : public MCTransition {
public:

  std::shared_ptr<MCRWWLock> rwwlock;
  MCRWWLockTransition(std::shared_ptr<MCThread> runner,
                      std::shared_ptr<MCRWWLock> rwwlock)
    : MCTransition(runner), rwwlock(rwwlock)
  {}
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKTRANSITION_HPP
