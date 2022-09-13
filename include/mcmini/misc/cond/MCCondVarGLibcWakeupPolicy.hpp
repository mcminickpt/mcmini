#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARGLIBCWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARGLIBCWAKEUPPOLICY_HPP

#include "mcmini/misc/cond/MCConditionVariableSignalPolicy.hpp"

namespace mcmini {

struct CondVarGLibcWakeupPolicy :
  public ConditionVariableSignalPolicy {

  CondVarGLibcWakeupPolicy() = default;
  CondVarGLibcWakeupPolicy(const CondVarGLibcWakeupPolicy &other) =
    default;

  std::unique_ptr<ConditionVariableSignalPolicy>
  clone() const override;

  void addWaiter(tid_t tid) override;
  void removeWaiter(tid_t tid) override;
  WakeGroup receiveSignalMessage() override;
  WakeGroup receiveBroadcastMessage() override;

private:

  void exchangeGroupsIfNecessary();

  std::vector<tid_t> group1;
  std::vector<tid_t> group2;
};

} // namespace mcmini

using MCCondVarGLibcWakeupPolicy = mcmini::CondVarGLibcWakeupPolicy;

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARGLIBCWAKEUPPOLICY_HPP
