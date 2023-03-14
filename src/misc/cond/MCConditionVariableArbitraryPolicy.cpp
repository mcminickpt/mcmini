#include "mcmini/misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include <algorithm>
#include <memory>

namespace mcmini {

std::unique_ptr<ConditionVariablePolicy>
ConditionVariableArbitraryPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableArbitraryPolicy>(
    new ConditionVariableArbitraryPolicy(*this));
}

void
ConditionVariableArbitraryPolicy::receive_signal_message()
{
  if (!this->sleep_queue.empty()) {
    this->wake_groups.push_back(
      WakeGroup(this->sleep_queue.begin(), this->sleep_queue.end()));
  }
}

} // namespace mcmini