#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKUNLOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKUNLOCK_HPP

#include "transitions/rwlock/MCRWLockTransition.h"

MCTransition *MCReadRWLockUnlock(const MCSharedTransition *, void *,
                                 MCStack *);

struct MCRWLockUnlock : public MCRWLockTransition {
public:

  MCRWLockUnlock(std::shared_ptr<MCThread> thread,
                 std::shared_ptr<MCRWLock> rwlock)
    : MCRWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKUNLOCK_HPP
