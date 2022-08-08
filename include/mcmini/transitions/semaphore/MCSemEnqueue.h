#ifndef MC_MCSEMENQUEUE_H
#define MC_MCSEMENQUEUE_H

#include "mcmini/transitions/semaphore/MCSemaphoreTransition.h"

MCTransition *MCReadSemEnqueue(const MCSharedTransition *, void *,
                               MCState *);

struct MCSemEnqueue : public MCSemaphoreTransition {
public:

  MCSemEnqueue(std::shared_ptr<MCThread> running,
               std::shared_ptr<MCSemaphore> sem)
    : MCSemaphoreTransition(running, sem)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  bool
  countsAgainstThreadExecutionDepth() const override
  {
    return false;
  }
  void print() const override;
};

#endif // MC_MCSEMENQUEUE_H
