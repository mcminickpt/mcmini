#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKTRANSITION_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKTRANSITION_HPP

#include "mcmini/MCTransition.h"
#include "mcmini/objects/MCRWLock.h"

struct MCRWLockTransition : public MCTransition {
public:

  std::shared_ptr<MCRWLock> rwlock;
  MCRWLockTransition(std::shared_ptr<MCThread> runner,
                     std::shared_ptr<MCRWLock> rwlock)
    : MCTransition(runner), rwlock(rwlock)
  {}
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKTRANSITION_HPP
