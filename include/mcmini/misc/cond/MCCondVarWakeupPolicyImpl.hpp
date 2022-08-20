#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARWAKEUPPOLICYIMPL_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARWAKEUPPOLICYIMPL_HPP

#include "mcmini/misc/cond/MCConditionVariableWakeupPolicy.hpp"
#include <list>

namespace mcmini {

struct CondVarWakeupPolicyImpl :
  public ConditionVariableWakeupPolicy {

  void pushWakeupGroup(const WakeGroup &group) override;
  void wakeThread(tid_t tid) override;

protected:

  std::list<WakeGroup> wakeGroups;
};

}; // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARWAKEUPPOLICYIMPL_HPP
