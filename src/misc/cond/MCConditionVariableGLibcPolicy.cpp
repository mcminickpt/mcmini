#include "mcmini/misc/cond/MCConditionVariableGLibcPolicy.hpp"
#include <algorithm>

namespace mcmini {

void
ConditionVariableGLibcPolicy::receive_signal_message()
{
  // TODO: Implement this
}

void
ConditionVariableGLibcPolicy::receive_broadcast_message()
{
  ConditionVariableDefaultPolicy::receive_broadcast_message();

  // Mark everyone as eligible for being woken up
  for (const tid_t sleeping_thread : this->group1) {
    this->broadcast_eligible_threads.insert(sleeping_thread);
  }
  for (const tid_t sleeping_thread : this->group2) {
    this->broadcast_eligible_threads.insert(sleeping_thread);
  }
  this->group1.clear();
  this->group2.clear();
}

void
ConditionVariableGLibcPolicy::wake_thread(tid_t tid)
{
  ConditionVariableDefaultPolicy::wake_thread(tid);

  // TODO: Implement this
}

void
ConditionVariableGLibcPolicy::add_waiter(tid_t tid)
{
  // TODO: Implement this
}

std::unique_ptr<ConditionVariablePolicy>
ConditionVariableGLibcPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableGLibcPolicy>(
    new ConditionVariableGLibcPolicy(*this));
}

} // namespace mcmini