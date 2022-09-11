#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERLOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERLOCK_HPP

#include "mcmini/transitions/rwlock/MCRWLockTransition.h"

MCTransition *MCReadRWLockWriterLock(const MCSharedTransition *,
                                     void *, MCState *);

struct MCRWLockWriterLock : public MCRWLockTransition {
public:

  MCRWLockWriterLock(std::shared_ptr<MCThread> thread,
                     std::shared_ptr<MCRWLock> rwlock)
    : MCRWLockTransition(thread, rwlock)
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

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKWRITERLOCK_HPP
