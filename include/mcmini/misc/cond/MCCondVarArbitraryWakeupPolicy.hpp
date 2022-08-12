#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP

#include "mcmini/misc/cond/MCConditionVariableWakeupPolicy.hpp"
#include <list>

namespace mcmini {

struct CondVarArbitraryWakeupPolicy :
  public ConditionVariableWakeupPolicy {

  CondVarArbitraryWakeupPolicy() = default;
  CondVarArbitraryWakeupPolicy(
    const CondVarArbitraryWakeupPolicy &other) = default;

  std::unique_ptr<ConditionVariableWakeupPolicy>
  clone() const override;

  void pushWakeupGroup(const WakeGroup &group) override;
  bool threadCanExit(tid_t tid) const override;
  void wakeThread(tid_t tid) override;

private:

  std::list<WakeGroup> wakeGroups;
};

}; // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
