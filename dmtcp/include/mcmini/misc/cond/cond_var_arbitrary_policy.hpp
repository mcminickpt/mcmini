#pragma once

#include "cond_var_single_grp_policy.hpp"
 
struct ConditionVariableArbitraryPolicy :
  public ConditionVariableSingleGroupPolicy {
    virtual void receive_signal_message() override;
    virtual bool has_waiters() const override;
    ConditionVariablePolicy* clone() const override; 
    std::deque<runner_id_t> return_wait_queue() const ;
    std::vector<WakeGroup> return_wake_groups() const ;
  };

using condition_variable_arbitrary_policy = 
  ConditionVariableArbitraryPolicy;