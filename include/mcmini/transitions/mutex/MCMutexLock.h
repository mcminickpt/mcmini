#ifndef MC_MCMUTEXLOCK_H
#define MC_MCMUTEXLOCK_H

#include "mcmini/transitions/mutex/MCMutexTransition.h"

MCTransition *MCReadMutexLock(const MCSharedTransition *, void *,
                              MCState *);

struct MCMutexLock : public MCMutexTransition {
public:

  MCMutexLock(std::shared_ptr<MCThread> thread,
              std::shared_ptr<MCMutex> mutex)
    : MCMutexTransition(thread, mutex)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  void unapplyToState(MCState *) override;
  bool isReversibleInState(const MCState *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCState *) const override;

  void print() const override;
};

#endif // MC_MCMUTEXLOCK_H
