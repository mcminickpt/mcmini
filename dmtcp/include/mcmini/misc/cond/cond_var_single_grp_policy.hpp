#include "cond_var_default_policy.hpp"
#include "mcmini/spy/checkpointing/objects.h"

#include <unordered_map>
#include <deque>

 struct ConditionVariableSingleGroupPolicy : 
 public ConditionVariableDefaultPolicy {
   virtual void receive_broadcast_message() override;
   virtual void wake_thread(runner_id_t tid) override;
   virtual void add_waiter(runner_id_t tid) override;
   virtual bool has_waiters() const;

 protected:
   std::deque<runner_id_t> wait_queue;
   std::unordered_map<runner_id_t, condition_variable_status> threads_with_states;
 };
