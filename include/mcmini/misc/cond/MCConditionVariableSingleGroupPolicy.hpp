#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLESINGLEGROUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLESINGLEGROUPPOLICY_HPP

#include "mcmini/misc/cond/MCConditionVariableDefaultPolicy.hpp"

#include <deque>

namespace mcmini {

struct ConditionVariableSingleGroupPolicy :
  public ConditionVariableDefaultPolicy {

  virtual void receive_broadcast_message() override;
  virtual void wake_thread(tid_t tid) override;
  virtual void add_waiter(tid_t tid) override;

protected:

  std::deque<tid_t> wait_queue;
};

} // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLESINGLEGROUPPOLICY_HPP
