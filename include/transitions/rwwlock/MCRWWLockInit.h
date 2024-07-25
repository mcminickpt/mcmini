#ifndef MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKINIT_H
#define MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKINIT_H

#include "transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockInit(const MCSharedTransition *, void *,
                                MCStack *);

struct MCRWWLockInit : public MCRWWLockTransition {
public:

  MCRWWLockInit(std::shared_ptr<MCThread> thread,
                std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKINIT_H
