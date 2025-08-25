#ifndef MC_MCMUTEXUNLOCK_H
#define MC_MCMUTEXUNLOCK_H

#include "transitions/mutex/MCMutexTransition.h"
#include <memory>

MCTransition *MCReadMutexUnlock(const MCSharedTransition *, void *,
                                MCStack *);

struct MCMutexUnlock : public MCMutexTransition {
public:

  MCMutexUnlock(std::shared_ptr<MCThread> thread,
                std::shared_ptr<MCMutex> mutex)
    : MCMutexTransition(thread, mutex)
  {}
  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  void unapplyToState(MCStack *) override;
  bool isReversibleInState(const MCStack *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // MC_MCMUTEXUNLOCK_H
