#ifndef MC_MCTHREADCREATE_H
#define MC_MCTHREADCREATE_H

#include "mcmini/MCShared.h"
#include "mcmini/transitions/threads/MCThreadTransition.h"

MCTransition *MCReadThreadCreate(const MCSharedTransition *, void *,
                                 MCState *);

struct MCThreadCreate : public MCThreadTransition {
public:
  inline MCThreadCreate(std::shared_ptr<MCThread> threadRunning,
                        std::shared_ptr<MCThread> threadCreated)
    : MCThreadTransition(threadRunning, threadCreated)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCState *) const override;
  void applyToState(MCState *) override;
  void unapplyToState(MCState *) override;
  bool isReversibleInState(const MCState *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;

  bool doesCreateThread(tid_t) const;
  void print() const override;
};

#endif // MC_MCTHREADCREATE_H
