#include "mcmini/misc/cond/MCConditionVariableOrderedPolicy.hpp"
#include <algorithm>
#include <memory>

namespace mcmini {

std::unique_ptr<ConditionVariablePolicy>
ConditionVariableOrderedPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableOrderedPolicy>(
    new ConditionVariableOrderedPolicy(*this));
}

void
ConditionVariableOrderedPolicy::receive_signal_message()
{
  if (!this->wait_queue.empty()) {
    switch (this->order) {
    case WakeOrder::fifo: {
      const WakeGroup wake_group{this->wait_queue.front()};
      this->wait_queue.pop_front();
      this->wake_groups.push_back(std::move(wake_group));
      break;
    }
    case WakeOrder::lifo: {
      const WakeGroup wake_group{this->wait_queue.back()};
      this->wait_queue.pop_back();
      this->wake_groups.push_back(std::move(wake_group));
      break;
    }
    }
  }
}

} // namespace mcmini