#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDVARSINGLEGROUPSIGNALPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDVARSINGLEGROUPSIGNALPOLICY_HPP

#include "mcmini/misc/cond/MCConditionVariableSignalPolicy.hpp"
#include <vector>

namespace mcmini {

struct CondVarSingleGroupSignalPolicy :
  public ConditionVariableSignalPolicy {

  CondVarSingleGroupSignalPolicy() = default;
  CondVarSingleGroupSignalPolicy(
    const CondVarSingleGroupSignalPolicy &other) = default;

  std::unique_ptr<ConditionVariableSignalPolicy>
  clone() const override;

  void addWaiter(tid_t tid) override;
  void removeWaiter(tid_t tid) override;
  WakeGroup receiveSignalMessage() override;
  WakeGroup receiveBroadcastMessage() override;

private:

  std::vector<tid_t> waiters;
};

} // namespace mcmini

using MCCondVarSingleGroupSignalPolicy =
  mcmini::CondVarSingleGroupSignalPolicy;

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDVARSINGLEGROUPSIGNALPOLICY_HPP
