#ifndef MC_MCMUTEXINIT_H
#define MC_MCMUTEXINIT_H

#include "mcmini/transitions/mutex/MCMutexTransition.h"

MCTransition *MCReadMutexInit(const MCSharedTransition *, void *,
                              MCState *);

struct MCMutexInit : public MCMutexTransition {
public:

  MCMutexInit(std::shared_ptr<MCThread> thread,
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
  void print() const override;
};

#endif // MC_MCMUTEXINIT_H
