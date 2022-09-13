#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP

#include "mcmini/misc/cond/MCCondVarWakeupPolicyImpl.hpp"

namespace mcmini {

struct CondVarArbitraryWakeupPolicy : public CondVarWakeupPolicyImpl {

  CondVarArbitraryWakeupPolicy() = default;
  CondVarArbitraryWakeupPolicy(
    const CondVarArbitraryWakeupPolicy &other) = default;

  std::unique_ptr<ConditionVariableWakeupPolicy>
  clone() const override;

  bool threadCanExit(tid_t tid) const override;
};

}; // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
