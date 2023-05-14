#ifndef MC_MCSEMWAIT_H
#define MC_MCSEMWAIT_H

#include "mcmini/transitions/semaphore/MCSemaphoreTransition.h"

MCTransition *MCReadSemWait(const MCSharedTransition *, void *,
                            MCStack *);

struct MCSemWait : public MCSemaphoreTransition {
public:

  MCSemWait(std::shared_ptr<MCThread> running,
            std::shared_ptr<MCSemaphore> sem)
    : MCSemaphoreTransition(running, sem)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCStack *) const override;
  void print() const override;
};

#endif // MC_MCSEMWAIT_H
