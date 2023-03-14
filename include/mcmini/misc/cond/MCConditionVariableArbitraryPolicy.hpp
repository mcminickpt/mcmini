#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP

#include "mcmini/misc/cond/MCConditionVariableDefaultPolicy.hpp"

namespace mcmini {

struct ConditionVariableArbitraryPolicy :
  public ConditionVariableDefaultPolicy {

  virtual void receive_signal_message() override;
  std::unique_ptr<ConditionVariablePolicy> clone() const override;
};

}; // namespace mcmini

using MCConditionVariableArbitraryPolicy =
  mcmini::ConditionVariableArbitraryPolicy;

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARARBITRARYWAKEUPPOLICY_HPP
