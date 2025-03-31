#pragma once

#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transitions/mutex/mutex_lock.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_signal.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_init.hpp"

namespace model {
namespace transitions {

struct condition_variable_wait : public model::transition {
 private:
  const state::objid_t cond_id; /* The condition variable this transition waits on */
  const state::objid_t mutex_id; /* The mutex this transition waits on */

 public:
  condition_variable_wait(runner_id_t executor, state::objid_t cond_id, state::objid_t mutex_id)
      : transition(executor), cond_id(cond_id), mutex_id(mutex_id) {}
  ~condition_variable_wait() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;

    // Retrive the state of the condition variable and the associated mutex
    const condition_variable* cv = s.get_state_of_object<condition_variable>(cond_id);
    const mutex* m = s.get_state_of_object<mutex>(mutex_id);

    if(cv->is_uninitialized() || cv->is_destroyed()) {
      return status::undefined;
    }

    // For the resumption phase, we now expect that the thread is currently waiting , so it should not already hold the mutex.
    if(m->is_locked_by(executor)) {
      return status::disabled;
    }

    if(!(cv->waiter_can_exit(executor) && m->get_location() == cv->get_mutex() && m->is_unlocked())){
      return status::disabled;
    }
    
    // condition_variable_status current_state = cv->get_policy()->get_thread_cv_state(executor);
    // if (current_state == CV_PREWAITING) {
    // // Thread not fully in wait state - update to WAITING before proceeding
    //   cv->get_policy()->update_thread_cv_state(executor, CV_WAITING);
    // }
    
    // Reacquire the mutex: update its state to "locked" with the executor.
    s.add_state_for_obj(mutex_id, new mutex(mutex::locked, m->get_location(), executor));

    // remove the executor from the wake group
    cv->remove_waiter(executor);
    
    const int new_waiting_count = cv->get_policy()->return_wait_queue().size();
    condition_variable::state new_state = new_waiting_count > 0
                                          ? condition_variable::cv_waiting
                                          : condition_variable::cv_signalled;
    s.add_state_for_obj(cond_id, new condition_variable(new_state, executor, m->get_location(), new_waiting_count));
    return status::exists;
  }
  state::objid_t get_id() const { return this->cond_id; }
  // std::string to_string() const override {
  //   return "pthread_cond_wait(cond:" + std::to_string(cond_id) + ", mutex:" + std::to_string(mutex_id) + ")";
  // }
  std::string to_string() const override {
    return "thread: " + std::to_string(executor) + " pthread_cond_wait (cond: )" + std::to_string(cond_id)
            + ", mutex: " + std::to_string(mutex_id) + ") (asleep -> awake)";
  }

  // MARK: Model checking functions
    bool depends(const condition_variable_init* ci) const {
        return this->cond_id == ci->get_id();
    }
    bool depends(const mutex_lock* ml) const {
        return this->mutex_id == ml->get_id();
    }
    bool coenabled_with(const condition_variable_wait* cw) const {
        return this->cond_id != cw->get_id();
    }
};
} // namespace transitions
} // namespace model
