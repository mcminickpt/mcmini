#include "mcmini/misc/cond/MCCondVarWakeupPolicyImpl.hpp"
#include <algorithm>
#include <memory>

using namespace mcmini;
using namespace std;

void
CondVarWakeupPolicyImpl::pushWakeupGroup(const WakeGroup &wg)
{
  // Maintains invariant that queued wake groups are non-empty
  if (!wg.empty()) this->wakeGroups.push_back(wg);
}

void
CondVarWakeupPolicyImpl::wakeThread(tid_t tid)
{
  for (WakeGroup &grp : wakeGroups) grp.removeCandidateThread(tid);
  const auto first =
    remove_if(wakeGroups.begin(), wakeGroups.end(),
              [](const WakeGroup &wg) -> bool { return wg.empty(); });
  wakeGroups.erase(first, wakeGroups.end());
}