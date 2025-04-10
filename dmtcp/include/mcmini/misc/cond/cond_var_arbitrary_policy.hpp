#pragma once

#include "cond_var_single_grp_policy.hpp"
// #include "mcmini/spy/checkpointing/objects.h"
 
struct ConditionVariableArbitraryPolicy :
  public ConditionVariableSingleGroupPolicy {
    virtual void receive_signal_message() override;
    virtual bool has_waiters() const override;
    void add_to_wake_groups (const std::vector<runner_id_t>& threads) override;
    void add_waiter_with_state(runner_id_t tid, condition_variable_status state) override;
    condition_variable_status get_thread_cv_state(runner_id_t tid) override;
    void update_thread_cv_state(runner_id_t tid, condition_variable_status state) override;
    ConditionVariablePolicy* clone() const override; 
    std::deque<runner_id_t> return_wait_queue() const ;
    std::vector<WakeGroup> return_wake_groups() const ;
  
  // protected:
  // std::unordered_map<runner_id_t, condition_variable_status> threads_with_states;
};

using condition_variable_arbitrary_policy = 
  ConditionVariableArbitraryPolicy;