#ifndef INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKREADERLOCK_HPP
#define INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKREADERLOCK_HPP

#include "mcmini/transitions/rwlock/MCRWLockTransition.h"

MCTransition *MCReadRWLockReaderLock(const MCSharedTransition *,
                                     void *, MCState *);

struct MCRWLockReaderLock : public MCRWLockTransition {
public:

  MCRWLockReaderLock(std::shared_ptr<MCThread> thread,
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

#endif // INCLUDE_MCMINI_TRANSITIONS_RWLOCK_MCRWLOCKREADERLOCK_HPP
