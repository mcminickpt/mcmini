#ifndef MC_MCTHREADSTART_H
#define MC_MCTHREADSTART_H

#include "MCShared.h"
#include "transitions/threads/MCThreadTransition.h"

MCTransition *MCReadThreadStart(const MCSharedTransition *, void *,
                                MCStack *);

struct MCThreadStart : public MCThreadTransition {
public:
  inline explicit MCThreadStart(std::shared_ptr<MCThread> thread)
    : MCThreadTransition(thread)
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
  bool countsAgainstThreadExecutionDepth() const override;
  void print() const override;
};

#endif // MC_MCTHREADSTART_H
