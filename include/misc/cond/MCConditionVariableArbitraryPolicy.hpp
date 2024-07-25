#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP

#include "misc/cond/MCConditionVariableSingleGroupPolicy.hpp"

namespace mcmini {

struct ConditionVariableArbitraryPolicy :
       public ConditionVariableSingleGroupPolicy {
  virtual void receive_signal_message() override;
  virtual bool has_waiters() const override;
  std::unique_ptr<ConditionVariablePolicy> clone() const override;
};

}; // namespace mcmini

using MCConditionVariableArbitraryPolicy =
  mcmini::ConditionVariableArbitraryPolicy;

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
