#include "mcmini/misc/cond/cond_var_arbitrary_policy.hpp"
#include <algorithm>
#include <memory>

ConditionVariablePolicy*
ConditionVariableArbitraryPolicy::clone() const
{
  return new ConditionVariableArbitraryPolicy(*this);
}

void
ConditionVariableArbitraryPolicy::receive_signal_message()
{
  if (!this->wait_queue.empty()) {
    this->wake_groups.push_back(
      WakeGroup(this->wait_queue.begin(), this->wait_queue.end()));
  }
}

bool
ConditionVariableArbitraryPolicy::has_waiters() const
{
  return ! this->wait_queue.empty();
}

std::deque<runner_id_t> ConditionVariableArbitraryPolicy::return_wait_queue() const {
  return this->wait_queue;
}

std::vector<WakeGroup> ConditionVariableArbitraryPolicy::return_wake_groups() const {
  return this->wake_groups;
}

