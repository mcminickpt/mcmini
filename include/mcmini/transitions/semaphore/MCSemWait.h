#ifndef MC_MCSEMWAIT_H
#define MC_MCSEMWAIT_H

#include "mcmini/transitions/semaphore/MCSemaphoreTransition.h"

MCTransition *MCReadSemWait(const MCSharedTransition *, void *,
                            MCState *);

struct MCSemWait : public MCSemaphoreTransition {
public:

  MCSemWait(std::shared_ptr<MCThread> running,
            std::shared_ptr<MCSemaphore> sem)
    : MCSemaphoreTransition(running, sem)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool enabledInState(const MCState *) const override;
  void print() const override;
};

#endif // MC_MCSEMWAIT_H
