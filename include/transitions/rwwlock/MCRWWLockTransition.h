#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKTRANSITION_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKTRANSITION_HPP

#include "MCTransition.h"
#include "objects/MCRWWLock.h"

struct MCRWWLockTransition : public MCTransition {
public:

  std::shared_ptr<MCRWWLock> rwwlock;
  MCRWWLockTransition(std::shared_ptr<MCThread> runner,
                      std::shared_ptr<MCRWWLock> rwwlock)
    : MCTransition(runner), rwwlock(rwwlock)
  {}

  std::unordered_set<objid_t> getObjectsAccessedByTransition() const override {
    return { rwwlock->getObjectId() };
  }
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKTRANSITION_HPP
