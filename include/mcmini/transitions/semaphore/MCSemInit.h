#ifndef MC_MCSEMINIT_H
#define MC_MCSEMINIT_H

#include "mcmini/transitions/semaphore/MCSemaphoreTransition.h"

MCTransition *MCReadSemInit(const MCSharedTransition *, void *,
                            MCState *);

struct MCSemInit : public MCSemaphoreTransition {
public:

  MCSemInit(std::shared_ptr<MCThread> running,
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

#endif // MC_MCSEMINIT_H
