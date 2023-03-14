#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEDEFAULTPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEDEFAULTPOLICY_HPP

#include "mcmini/misc/cond/MCConditionVariablePolicy.hpp"
#include "mcmini/misc/cond/MCWakeGroup.hpp"

#include <unordered_set>
#include <vector>

namespace mcmini {

/*
 * Threads are queued by the wakeup policy into *wake groups*. A
 * wake group describes a mutually-exclusive set of threads which
 * can escape the condition variable. See the documentation for
 * `mcmini::WakeGroup` for more details.
 */
struct ConditionVariableDefaultPolicy :
  public ConditionVariablePolicy {

  virtual void receive_broadcast_message() override;
  virtual bool thread_can_exit(tid_t tid) const override;
  virtual void wake_thread(tid_t tid) override;
  virtual void add_waiter(tid_t tid) override;

protected:

  std::unordered_set<tid_t> broadcast_eligible_threads;
  std::vector<WakeGroup> wake_groups;
  std::vector<tid_t> sleep_queue;
};

}; // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLEDEFAULTPOLICY_HPP
