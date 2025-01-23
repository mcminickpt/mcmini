#include "cond_var_wakegroup.hpp"
#include "cond_var_policy.hpp"

#include <deque>
#include <unordered_set>
#include <vector>

/*
 * Threads are queued by the wakeup policy into *wake groups*. A
 * wake group describes a mutually-exclusive set of threads which
 * can escape the condition variable. See the documentation for
 * `mcmini::WakeGroup` for more details.
 */
struct ConditionVariableDefaultPolicy : 
  public ConditionVariablePolicy {
    virtual void receive_broadcast_message() override;
    virtual bool thread_can_exit(runner_id_t tid) const override;
    virtual void wake_thread(runner_id_t tid) override;
    virtual bool has_waiters() const;
  
  protected:
    std::unordered_set<runner_id_t> broadcast_eligible_threads;
    std::vector<WakeGroup> wake_groups;
  };