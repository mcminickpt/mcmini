#include "mcmini/misc/cond/MCConditionVariableSingleGroupPolicy.hpp"

#include <algorithm>

namespace mcmini {

void
ConditionVariableSingleGroupPolicy::receive_broadcast_message()
{
  // Empty BOTH the sleep queue and the wake group list:
  // at this point everyone is allowed to wake up
  ConditionVariableDefaultPolicy::receive_broadcast_message();

  for (const tid_t waiting_thread : this->wait_queue) {
    this->broadcast_eligible_threads.insert(waiting_thread);
  }
  this->wait_queue.clear();
}

void
ConditionVariableSingleGroupPolicy::wake_thread(tid_t tid)
{
  ConditionVariableDefaultPolicy::wake_thread(tid);

  // Remove the thread from the sleep queue if it is still
  // contained in there. Some policies may decide to keep
  // the thread in the queue as part of their implementation,
  // but they should always be removed when a thread is woken
  const auto waiting_thread =
    std::find(wait_queue.begin(), wait_queue.end(), tid);
  if (waiting_thread != wait_queue.end()) {
    wait_queue.erase(waiting_thread);
  }
}

void
ConditionVariableSingleGroupPolicy::add_waiter(tid_t tid)
{
  this->wait_queue.push_back(tid);
}

} // namespace mcmini
