#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKUNLOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKUNLOCK_HPP

#include "transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockUnlock(const MCSharedTransition *, void *,
                                  MCStack *);

struct MCRWWLockUnlock : public MCRWWLockTransition {
public:

  MCRWWLockUnlock(std::shared_ptr<MCThread> thread,
                  std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKUNLOCK_HPP
