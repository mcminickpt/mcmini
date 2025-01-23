#include "dmtcp/include/mcmini/misc/cond/cond_var_arbitrary_policy.hpp"
#include <algorithm>
#include <memory>



std::unique_ptr<ConditionVariablePolicy>
ConditionVariableArbitraryPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableArbitraryPolicy>(
    new ConditionVariableArbitraryPolicy(*this));
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

