#ifndef MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKINIT_H
#define MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKINIT_H

#include "mcmini/transitions/rwwlock/MCRWWLockTransition.h"

MCTransition *MCReadRWWLockInit(const MCSharedTransition *, void *,
                                MCState *);

struct MCRWWLockInit : public MCRWWLockTransition {
public:

  MCRWWLockInit(std::shared_ptr<MCThread> thread,
                std::shared_ptr<MCRWWLock> rwlock)
    : MCRWWLockTransition(thread, rwlock)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // MCMINI_TRANSITIONS_RWWLOCK_MCRWWLOCKINIT_H
