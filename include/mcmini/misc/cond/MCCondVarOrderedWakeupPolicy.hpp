#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARORDEREDWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARORDEREDWAKEUPPOLICY_HPP

#include "mcmini/misc/cond/MCCondVarWakeupPolicyImpl.hpp"

namespace mcmini {

struct CondVarOrderedWakeupPolicy : public CondVarWakeupPolicyImpl {

  enum class WakeupOrder {
    fifo,
    lifo
  } wakeupOrder = WakeupOrder::fifo;

  CondVarOrderedWakeupPolicy() = default;
  CondVarOrderedWakeupPolicy(WakeupOrder wakeupOrder)
    : wakeupOrder(wakeupOrder)
  {}
  CondVarOrderedWakeupPolicy(
    const CondVarOrderedWakeupPolicy &other) = default;

  std::unique_ptr<ConditionVariableWakeupPolicy>
  clone() const override;

  bool threadCanExit(tid_t tid) const override;
};

}; // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARORDEREDWAKEUPPOLICY_HPP
