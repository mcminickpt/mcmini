#ifndef MC_MCMUTEXTRYLOCK_H
#define MC_MCMUTEXTRYLOCK_H

#include "transitions/mutex/MCMutexTransition.h"
#include <memory>

MCTransition *MCReadMutexTrylock(const MCSharedTransition *, void *,
                                 MCStack *);

struct MCMutexTrylock : public MCMutexTransition {
public:
  // true  -> pthread_mutex_trylock returned 0
  // false -> pthread_mutex_trylock returned EBUSY
  bool acquired;

  MCMutexTrylock(std::shared_ptr<MCThread> thread,
                 std::shared_ptr<MCMutex> mutex,
                 bool acquired)
    : MCMutexTransition(thread, mutex), acquired(acquired)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  void unapplyToState(MCStack *) override;
  bool isReversibleInState(const MCStack *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCStack *) const override;

  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // MC_MCMUTEXTRYLOCK_H
