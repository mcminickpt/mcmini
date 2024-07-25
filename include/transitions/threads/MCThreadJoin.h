#ifndef MC_MCTHREADJOIN_H
#define MC_MCTHREADJOIN_H

#include "MCShared.h"
#include "transitions/threads/MCThreadTransition.h"

MCTransition *MCReadThreadJoin(const MCSharedTransition *, void *,
                               MCStack *);

struct MCThreadJoin : public MCThreadTransition {
public:
  inline MCThreadJoin(std::shared_ptr<MCThread> threadRunning,
                      std::shared_ptr<MCThread> joinedOn)
    : MCThreadTransition(threadRunning, joinedOn)
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
  bool enabledInState(const MCStack *) const override;

  bool joinsOnThread(tid_t) const;
  bool joinsOnThread(const std::shared_ptr<MCThread> &) const;

  void print() const override;
};

#endif // MC_MCTHREADJOIN_H
