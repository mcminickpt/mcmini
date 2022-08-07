#ifndef MC_MCCONDBROADCAST_H
#define MC_MCCONDBROADCAST_H

#include "mcmini/transitions/cond/MCCondTransition.h"

MCTransition *MCReadCondBroadcast(const MCSharedTransition *, void *,
                                  MCState *);

struct MCCondBroadcast : public MCCondTransition {
public:
  MCCondBroadcast(std::shared_ptr<MCThread> running,
                  std::shared_ptr<MCConditionVariable> cond)
    : MCCondTransition(running, cond)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;
  void print() const override;
};

#endif // MC_MCCONDBROADCAST_H
