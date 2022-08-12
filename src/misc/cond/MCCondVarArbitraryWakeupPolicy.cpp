#include "mcmini/misc/cond/MCCondVarArbitraryWakeupPolicy.hpp"
#include <algorithm>
#include <memory>

using namespace mcmini;
using namespace std;

unique_ptr<ConditionVariableWakeupPolicy>
CondVarArbitraryWakeupPolicy::clone() const
{
  return std::unique_ptr<CondVarArbitraryWakeupPolicy>(
    new CondVarArbitraryWakeupPolicy(*this));
}

void
CondVarArbitraryWakeupPolicy::pushWakeupGroup(const WakeGroup &wg)
{
  this->wakeGroups.push_back(wg);
}

bool
CondVarArbitraryWakeupPolicy::threadCanExit(tid_t tid) const
{
  // return any_of(
  //   wakeGroups.begin(), wakeGroups.end(),
  //   [tid](const WakeGroup &wg) { return wg.containsThread(tid); });
  return false;
}

void
CondVarArbitraryWakeupPolicy::wakeThread(tid_t tid)
{
  for (WakeGroup &grp : wakeGroups) grp.removeCandidateThread(tid);
  const auto first =
    remove_if(wakeGroups.begin(), wakeGroups.end(),
              [](const WakeGroup &wg) -> bool { return wg.empty(); });
  wakeGroups.erase(first, wakeGroups.end());
}