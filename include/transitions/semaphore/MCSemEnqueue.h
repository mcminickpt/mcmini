#ifndef MC_MCSEMENQUEUE_H
#define MC_MCSEMENQUEUE_H

#include "transitions/semaphore/MCSemaphoreTransition.h"

MCTransition *MCReadSemEnqueue(const MCSharedTransition *, void *,
                               MCStack *);

struct MCSemEnqueue : public MCSemaphoreTransition {
public:

  MCSemEnqueue(std::shared_ptr<MCThread> running,
               std::shared_ptr<MCSemaphore> sem)
    : MCSemaphoreTransition(running, sem)
  {}

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
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
