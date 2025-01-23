#include "dmtcp/include/mcmini/misc/cond/cond_var_single_grp_policy.hpp"
 
struct ConditionVariableArbitraryPolicy :
  public ConditionVariableSingleGroupPolicy {
    virtual void receive_signal_message() override;
    virtual bool has_waiters() const override;
    std::unique_ptr<ConditionVariablePolicy> clone() const override; 
  };

using condition_variable_arbitrary_policy = 
  ConditionVariableArbitraryPolicy;