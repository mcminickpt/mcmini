#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKWRITER2ENQUEUE_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKWRITER2ENQUEUE_HPP

#include "transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockWriter2Enqueue(const MCSharedTransition *,
                                          void *, MCStack *);

struct MCRWWLockWriter2Enqueue : public MCRWWLockTransition {
public:

  MCRWWLockWriter2Enqueue(std::shared_ptr<MCThread> thread,
                          std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool
  countsAgainstThreadExecutionDepth() const override
  {
    return false;
  }
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKWRITER2ENQUEUE_HPP
