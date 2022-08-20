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

bool
CondVarArbitraryWakeupPolicy::threadCanExit(tid_t tid) const
{
  return any_of(
    wakeGroups.begin(), wakeGroups.end(),
    [tid](const WakeGroup &wg) { return wg.containsThread(tid); });
}