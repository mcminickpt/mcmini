#pragma once

#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transitions/mutex/mutex_lock.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_init.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_signal.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_wait.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variable_enqueue_thread.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_destroy.hpp"

namespace model {
namespace transitions {

struct condition_variable_broadcast : public model::transition {
 private:
  const state::objid_t cond_id;
  mutable bool had_waiters = false;

 public:
  condition_variable_broadcast(runner_id_t executor, state::objid_t cond_id)
    : transition(executor), cond_id(cond_id) {}
  ~condition_variable_broadcast() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;

    // Retrieve the state of the condition variable
    const condition_variable* cv = s.get_state_of_object<condition_variable>(cond_id);
    if (cv->is_uninitialized()) {
      return status::undefined;
    }
    if (cv->is_destroyed()) {
      return status::undefined;
    }
    
    // Check if there are waiters (if not, broadcast is a no-op but still valid)
    if (!cv->has_waiters()) {
      return status::exists; 
    }

    // Find all CV_WAITING threads (not CV_PREWAITING)
    std::vector<runner_id_t> waiting_threads;
    const auto& wait_queue = cv->get_policy()->return_wait_queue();
    for (auto tid : wait_queue) {
      if (cv->get_policy()->get_thread_cv_state(tid) == CV_WAITING) {
        waiting_threads.push_back(tid);
      }
    }

    // Add only CV_WAITING threads to wake groups
    if (!waiting_threads.empty()) {
      cv->get_policy()->add_to_wake_groups(waiting_threads);
    }
    
    // Make the threads in the wake groups eligible to be woken up by adding them all to broadcast_eligible_threads
    cv->send_broadcast_message();
    
    // Update condition variable state
    const int new_waiting_count = cv->get_policy()->return_wait_queue().size();
    condition_variable::state new_state = new_waiting_count > 0
                                        ? condition_variable::cv_waiting
                                        : condition_variable::cv_signaled;
                          
    s.add_state_for_obj(cond_id, new condition_variable(new_state, executor, 
                                                        cv->get_mutex(),
                                                        new_waiting_count));
    return status::exists;
  }
  state::objid_t get_id() const { return this->cond_id;}
  std::string to_string() const override {
    return "pthread_cond_broadcast(condition_variable:" + std::to_string(cond_id) + ")";
  }

  // MARK: Model checking functions
  bool coenabled_with(const condition_variable_wait* cw) const {
    return true; // Broadcast can be co-enabled with a wait operation
  }

  bool coenabled_with(const mutex_unlock* mu) const {
    return true; // Broadcast can happen concurrently with mutex unlock
  }

  bool coenabled_with(const condition_variable_destroy* cd) const {
    return this->cond_id != cd->get_id(); // Can't broadcast and destroy same CV
  }

  bool depends(const condition_variable_wait* cw) const {
    return this->cond_id == cw->get_id(); // Dependent with wait on same CV
  }

  bool depends(const condition_variable_signal* cs) const {
    return this->cond_id == cs->get_id(); // Dependent with signal on same CV
  }

  bool depends(const condition_variable_destroy* cd) const {
    return this->cond_id == cd->get_id(); // Dependent with destroy on same CV
  }
};
} // namespace transitions
} // namespace model
