#pragma once

#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_wait.hpp"
#include "mcmini/model/transitions/mutex/mutex_unlock.hpp"

namespace model {
namespace transitions {

struct condition_variable_signal : public model::transition {
 private:
  const state::objid_t cond_id; /* The condition variable this transition signals */
  mutable bool had_waiters = false;
  state::objid_t get_objid_by_location(const mutable_state& s, pthread_mutex_t* mutex_location) const {
    
    for (state::objid_t id = 0; id < invalid_objid; id++){
      if(!s.contains_object_with_id(id)) continue;

      // const visible_object_state* obj = s.state::get_state_of_object(id);
      const visible_object_state* obj = s.get_state_of_object<visible_object_state>(id);
      const objects::mutex* m = dynamic_cast<const objects::mutex*>(obj);
      if(m && m->get_location() == mutex_location){
        return id;
      }
    }
    return invalid_objid;
  }

  public:
   condition_variable_signal(runner_id_t executor, state::objid_t cond_id)
       : transition(executor), cond_id(cond_id) {}
  ~condition_variable_signal() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;

    // Retrive the state of the condition variable
    const condition_variable* cv = s.get_state_of_object<condition_variable>(cond_id);

    if(cv->is_uninitialized()) {
      return status::undefined;
    }
    // Ensure that the condition variable is in the waiting state
    // if (!cv->is_waiting()) {
    //   return status::disabled;
    // }
    //instead of disable add another status "undefined"
    if(cv->is_destroyed()){
      return status::undefined;
    }
    // pthread_mutex_t* mutex_location = cv->get_mutex();

    // if(!mutex_location) {
    //   return status::undefined;
    // }

    // const state::objid_t mutex_id = get_objid_by_location(s, mutex_location);
    // const mutex* m = s.get_state_of_object<mutex>(mutex_id);
    // if(!m || m->is_destroyed()){
    //   return status::undefined;
    // }
    
    //Check if there are waiters (if not, signal is a no-op but still valid)
    if(!cv->has_waiters()) {
      return status::exists; //valid transition (lost wakeup)
    }
    // Receive the signal message (move waiters to wake_groups)
    cv->send_signal_message();

    //Find the first eligible thread to wake up
    // runner_id_t thread_to_wake;
    // bool found_eligible_thread = false; 
    // const auto& wake_groups = cv->get_policy()->return_wake_groups();
    // for(const auto& group: wake_groups) {
    //   for(const auto& thread: group){
    //   // // Verify mutex can be acquired
    //   // const mutex* m = s.get_state_of_object<mutex>(mutex_id);
    //   // if(m && m->is_unlocked()){
    //     thread_to_wake = thread;
    //     found_eligible_thread = true;
    //     break;
    //   // }
    // }
    // }
    //  if (!found_eligible_thread) {
    //   return status::exists; // No eligible thread to wake up
    // }

    // // Update mutex state
    // s.add_state_for_obj(mutex_id, new mutex(mutex::locked, mutex_location, thread_to_wake));

    // Wake the thread (remove from wait_queue and update policy)
    // cv->get_policy()->wake_thread(thread_to_wake);
    // cv->remove_waiter(thread_to_wake);

    // Update the condition variable state
    const int new_waiting_count = cv->get_policy()->return_wait_queue().size();
    condition_variable::state new_state = new_waiting_count > 0
                                          ? condition_variable::cv_waiting
                                          : condition_variable::cv_signalled;
                            
    s.add_state_for_obj(cond_id, new condition_variable(new_state, new_waiting_count));
    return status::exists;   
  }

  state::objid_t get_id() const { return this->cond_id; }
  std::string to_string() const override {
    return "pthread_cond_signal(cond:" + std::to_string(cond_id) + ")";
  }
// MARK: Model checking functions
  bool depends(const condition_variable_wait* cw) const {
    return this->cond_id == cw->get_id();
  }
  bool depends(const mutex_lock* ml) const {
    return this->cond_id == ml->get_id();
  }
  bool coenabled_with(const mutex_unlock* mu) const {
    return this->cond_id != mu->get_id();
  }
  bool coenabled_with(const condition_variable_wait* cw) const {
    return this->cond_id != cw->get_id();
  }

};
} // namespace transitions
} // namespace model
