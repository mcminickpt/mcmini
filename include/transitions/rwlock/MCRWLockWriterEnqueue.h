#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERENQUEUE_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERENQUEUE_HPP

#include "transitions/rwlock/MCRWLockTransition.h"

MCTransition *MCReadRWLockWriterEnqueue(const MCSharedTransition *,
                                        void *, MCStack *);

struct MCRWLockWriterEnqueue : public MCRWLockTransition {

  MCRWLockWriterEnqueue(std::shared_ptr<MCThread> thread,
                        std::shared_ptr<MCRWLock> rwlock)
    : MCRWLockTransition(thread, rwlock)
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

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERENQUEUE_HPP
