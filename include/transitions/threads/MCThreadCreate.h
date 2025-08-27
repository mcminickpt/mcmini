#ifndef MC_MCTHREADCREATE_H
#define MC_MCTHREADCREATE_H

#include "MCShared.h"
#include "transitions/threads/MCThreadTransition.h"

MCTransition *MCReadThreadCreate(const MCSharedTransition *, void *,
                                 MCStack *);

struct MCThreadCreate : public MCThreadTransition {
public:
  inline MCThreadCreate(std::shared_ptr<MCThread> threadRunning,
                        std::shared_ptr<MCThread> threadCreated)
    : MCThreadTransition(threadRunning, threadCreated)
  {
  }

  std::shared_ptr<MCTransition> staticCopy() const override;
  std::shared_ptr<MCTransition>
  dynamicCopyInState(const MCStack *) const override;
  void applyToState(MCStack *) override;
  void unapplyToState(MCStack *) override;
  bool isReversibleInState(const MCStack *) const override;
  bool coenabledWith(const MCTransition *) const override;
  bool dependentWith(const MCTransition *) const override;

  bool doesCreateThread(tid_t) const;
  MCTransitionUniqueRep toUniqueRep() const override;
  void print() const override;
};

#endif // MC_MCTHREADCREATE_H
