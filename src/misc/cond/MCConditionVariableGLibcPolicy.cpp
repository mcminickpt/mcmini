#include "mcmini/misc/cond/MCConditionVariableGLibcPolicy.hpp"
#include <algorithm>

namespace mcmini {

void
ConditionVariableGLibcPolicy::receive_broadcast_message()
{
  ConditionVariableDefaultPolicy::receive_broadcast_message();

  // Mark everyone as eligible for being woken up
  for (const tid_t waiting_thread : this->group1) {
    this->broadcast_eligible_threads.insert(waiting_thread);
  }
  for (const tid_t waiting_thread : this->group2) {
    this->broadcast_eligible_threads.insert(waiting_thread);
  }
  this->group1.clear();
  this->group2.clear();
}

void
ConditionVariableGLibcPolicy::receive_signal_message()
{
  // Threads in group 1, when signaled, are allowed
  // to escape from the condition variable arbitrarily.
  //
  // Otherwise, if group 1 is empty, group 2 becomes the
  // new group 1 and we wake threads arbitrarily as before.
  if (group1.empty()) {
    this->group1 = this->group2;
    this->group2.clear();
  }

  // It's possible that group2 was empty to, in which case there's
  // nothing to do and the signal goes unconsumed.
  if (!group1.empty()) {
    this->wake_groups.push_back(
      WakeGroup(group1.begin(), group1.end()));
  }
}

void
ConditionVariableGLibcPolicy::wake_thread(tid_t tid)
{
  ConditionVariableDefaultPolicy::wake_thread(tid);

  // Remove the thread from the first group
  const auto thread_in_group1 =
    std::find(group1.begin(), group1.begin(), tid);

  // It's possible that the thread is being woken
  // but is NOT contained in group1 (e.g. after a
  // broadcast where the thread is moved into
  // the set of `broadcast_eligible_threads`).
  // If it is actually contained in the group,
  // we remove it in case it decides to reenter.
  if (thread_in_group1 == group1.end()) {
    group1.erase(thread_in_group1);
  }

  {
    // The thread should NOT be in group 2: only threads
    // from group 1 should be eligible to wake up.
    const auto thread_in_group2 =
      std::find(group2.begin(), group2.begin(), tid);
    MC_ASSERT(thread_in_group2 == group2.end());
  }
}

void
ConditionVariableGLibcPolicy::add_waiter(tid_t tid)
{
  // New threads enter into group 2 in the glibc scheme.
  this->group2.push_back(tid);
}

std::unique_ptr<ConditionVariablePolicy>
ConditionVariableGLibcPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableGLibcPolicy>(
    new ConditionVariableGLibcPolicy(*this));
}

} // namespace mcmini