#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEORDEREDPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEORDEREDPOLICY_HPP

#include "misc/cond/MCConditionVariableSingleGroupPolicy.hpp"

namespace mcmini {

struct ConditionVariableOrderedPolicy :
  public ConditionVariableSingleGroupPolicy {

  /// @brief Whether the wake up ordering is fifo or lifo
  enum class WakeOrder { fifo, lifo } order = WakeOrder::fifo;

  ConditionVariableOrderedPolicy() = default;
  explicit ConditionVariableOrderedPolicy(WakeOrder order)
    : order(order)
  {}

  virtual void receive_signal_message() override;
  std::unique_ptr<ConditionVariablePolicy> clone() const override;
};

}; // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEORDEREDPOLICY_HPP
