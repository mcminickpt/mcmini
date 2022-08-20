#include "mcmini/misc/cond/MCCondVarOrderedWakeupPolicy.hpp"
#include <algorithm>
#include <memory>

using namespace mcmini;
using namespace std;

unique_ptr<ConditionVariableWakeupPolicy>
CondVarOrderedWakeupPolicy::clone() const
{
  return std::unique_ptr<CondVarOrderedWakeupPolicy>(
    new CondVarOrderedWakeupPolicy(*this));
}

bool
CondVarOrderedWakeupPolicy::threadCanExit(tid_t tid) const
{
  if (wakeGroups.empty()) return false;

  if (this->wakeupOrder == WakeupOrder::fifo) {
    // FIFO wake ups are easy: simply check if the
    // candidate thread is the first thread in any
    // wake group/ This works assuming that the invariant
    // holds (see the discussion in the ConditionVariableWakeupPolicy
    // header).
    const WakeGroup &firstWG = wakeGroups.front();

    // NOTE: No queued wake group should be empty by the
    // invariant maintained by this wake up policy
    MC_ASSERT(!firstWG.empty());

    MCOptional<tid_t> thrdind = firstWG.top();
    MC_ASSERT(thrdind.hasValue());
    return thrdind.unwrapped() == tid;
  } else {

    // LIFO is a bit trickier. With LIFO, it's possible that a thread
    // could be the last thread to have entered the condition variable
    // at the time of one signal, but may no longer be the last
    // candidate thread at a later time when another signal is sent to
    // the condition variable
    //
    // HOWEVER, under the invariant, the thread which will wake up
    // next from the condition variable MUST be the LAST thread in the
    // LAST (non-empty)wake group; otherwise there would be an
    // elligible thread which entered after `tid` which hasn't yet
    // awoken
    const WakeGroup &lastWG = wakeGroups.back();

    // NOTE: No queued wake group should be empty by the
    // invariant maintained by this wake up policy
    MC_ASSERT(!lastWG.empty());

    MCOptional<tid_t> thrdind = lastWG.back();
    MC_ASSERT(thrdind.hasValue());
    return thrdind.unwrapped() == tid;
  }
}