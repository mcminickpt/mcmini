#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKWRITER2LOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKWRITER2LOCK_HPP

#include "mcmini/transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockWriter2Lock(const MCSharedTransition *,
                                       void *, MCState *);

struct MCRWWLockWriter2Lock : public MCRWWLockTransition {
public:

  MCRWWLockWriter2Lock(std::shared_ptr<MCThread> thread,
                       std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool enabledInState(const MCState *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKWRITER2LOCK_HPP
