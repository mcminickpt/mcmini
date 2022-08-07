#ifndef MC_MCTHREADJOIN_H
#define MC_MCTHREADJOIN_H

#include "mcmini/MCShared.h"
#include "mcmini/transitions/threads/MCThreadTransition.h"

MCTransition *MCReadThreadJoin(const MCSharedTransition *, void *,
                               MCState *);

struct MCThreadJoin : public MCThreadTransition {
public:
  inline MCThreadJoin(std::shared_ptr<MCThread> threadRunning,
                      std::shared_ptr<MCThread> joinedOn)
    : MCThreadTransition(threadRunning, joinedOn)
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
  bool enabledInState(const MCState *) const override;

  bool joinsOnThread(tid_t) const;
  bool joinsOnThread(const std::shared_ptr<MCThread> &) const;

  void print() const override;
};

#endif // MC_MCTHREADJOIN_H
