#ifndef MC_MCSEMPOST_H
#define MC_MCSEMPOST_H

#include "mcmini/transitions/semaphore/MCSemaphoreTransition.h"

MCTransition *MCReadSemPost(const MCSharedTransition *, void *,
                            MCState *);

struct MCSemPost : public MCSemaphoreTransition {
public:

  MCSemPost(std::shared_ptr<MCThread> running,
            std::shared_ptr<MCSemaphore> sem)
    : MCSemaphoreTransition(running, sem)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // MC_MCSEMPOST_H
