#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERLOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERLOCK_HPP

#include "transitions/rwlock/MCRWLockTransition.h"

MCTransition *MCReadRWLockWriterLock(const MCSharedTransition *,
                                     void *, MCStack *);

struct MCRWLockWriterLock : public MCRWLockTransition {
public:

  MCRWLockWriterLock(std::shared_ptr<MCThread> thread,
                     std::shared_ptr<MCRWLock> rwlock)
    : MCRWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool enabledInState(const MCStack *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERLOCK_HPP
