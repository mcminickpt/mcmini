#include "mcmini/misc/cond/MCConditionVariableSingleGroupPolicy.hpp"

#include <algorithm>

namespace mcmini {

void
ConditionVariableSingleGroupPolicy::receive_broadcast_message()
{
  // Empty BOTH the sleep queue and the wake group list:
  // at this point everyone is allowed to wake up
  ConditionVariableDefaultPolicy::receive_broadcast_message();

  for (const tid_t sleeping_thread : this->sleep_queue) {
    this->broadcast_eligible_threads.insert(sleeping_thread);
  }
  this->sleep_queue.clear();
}

void
ConditionVariableSingleGroupPolicy::wake_thread(tid_t tid)
{
  ConditionVariableDefaultPolicy::wake_thread(tid);

  // Remove the thread from the sleep queue if it is still
  // contained in there. Some policies may decide to keep
  // the thread in the queue as part of their implementation,
  // but they should always be removed when a thread is awoken
  const auto sleeping_thread =
    std::find(sleep_queue.begin(), sleep_queue.end(), tid);
  if (sleeping_thread != sleep_queue.end()) {
    sleep_queue.erase(sleeping_thread);
  }
}

void
ConditionVariableSingleGroupPolicy::add_waiter(tid_t tid)
{
  this->sleep_queue.push_back(tid);
}

} // namespace mcmini