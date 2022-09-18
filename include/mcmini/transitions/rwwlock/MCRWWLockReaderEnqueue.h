#ifndef INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKREADERENQUEUE_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKREADERENQUEUE_HPP

#include "mcmini/transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockReaderEnqueue(const MCSharedTransition *,
                                         void *, MCState *);

struct MCRWWLockReaderEnqueue : public MCRWWLockTransition {
public:

  MCRWWLockReaderEnqueue(std::shared_ptr<MCThread> thread,
                         std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
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

#endif // INCLUDE_MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKREADERENQUEUE_HPP
