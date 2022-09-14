#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKREADERENQUEUE_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKREADERENQUEUE_HPP

#include "mcmini/transitions/rwlock/MCRWLockTransition.h"

MCTransition *MCReadRWLockReaderEnqueue(const MCSharedTransition *,
                                        void *, MCState *);

struct MCRWLockReaderEnqueue : public MCRWLockTransition {

  MCRWLockReaderEnqueue(std::shared_ptr<MCThread> thread,
                        std::shared_ptr<MCRWLock> rwlock)
    : MCRWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool
  countsAgainstThreadExecutionDepth() const override
  {
    return false;
  }
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKREADERENQUEUE_HPP
