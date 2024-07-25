#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEGLIBCPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEGLIBCPOLICY_HPP

#include "misc/cond/MCConditionVariableDefaultPolicy.hpp"

#include <deque>

namespace mcmini {

struct ConditionVariableGLibcPolicy :
  public ConditionVariableDefaultPolicy {

  virtual void receive_signal_message() override;
  virtual void receive_broadcast_message() override;
  virtual void wake_thread(tid_t tid) override;
  virtual void add_waiter(tid_t tid) override;
  virtual bool has_waiters() const override;
  std::unique_ptr<ConditionVariablePolicy> clone() const override;

private:

  std::deque<tid_t> group1;
  std::deque<tid_t> group2;
};

} // namespace mcmini

using MCCondVarGLibcWakeupPolicy =
  mcmini::ConditionVariableGLibcPolicy;

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEGLIBCPOLICY_HPP
