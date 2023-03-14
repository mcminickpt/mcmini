#include "mcmini/misc/cond/MCConditionVariableDefaultPolicy.hpp"
#include <algorithm>
#include <memory>

namespace mcmini {

void
ConditionVariableDefaultPolicy::receive_broadcast_message()
{
  // Move everyone into the get-out-of-jail free place
}

bool
ConditionVariableDefaultPolicy::thread_can_exit(tid_t tid) const
{
  return false;
}

void
ConditionVariableDefaultPolicy::wake_thread(tid_t tid)
{}

void
ConditionVariableDefaultPolicy::add_waiter(tid_t tid)
{}

} // namespace mcmini