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

void ConditionVariableArbitraryPolicy::add_to_wake_groups(const std::vector<runner_id_t>& threads) {
  if (!threads.empty()) {
    this->wake_groups.push_back(WakeGroup(threads.begin(), threads.end()));
  }
}

// Add thread with specific state
void ConditionVariableArbitraryPolicy::add_waiter_with_state(runner_id_t tid, condition_variable_status state) {
  add_waiter(tid);
  this->threads_with_states[tid] = state;
}

// Get thread's current state
condition_variable_status ConditionVariableArbitraryPolicy::get_thread_cv_state(runner_id_t tid) {
  auto it = this->threads_with_states.find(tid);
  return (it != this->threads_with_states.end()) ? it->second : CV_UNINITIALIZED;
}

void ConditionVariableArbitraryPolicy::update_thread_cv_state(runner_id_t tid, condition_variable_status state) {
  this->threads_with_states[tid] = state;
}


