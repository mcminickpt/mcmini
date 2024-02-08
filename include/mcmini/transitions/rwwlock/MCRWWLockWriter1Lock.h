#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCWRITER1LOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCWRITER1LOCK_HPP

#include "mcmini/transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockWriter1Lock(const MCSharedTransition *,
                                       void *, MCStack *);

struct MCRWWLockWriter1Lock : public MCRWWLockTransition {
public:

  MCRWWLockWriter1Lock(std::shared_ptr<MCThread> thread,
                       std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool enabledInState(const MCStack *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCWRITER1LOCK_HPP
