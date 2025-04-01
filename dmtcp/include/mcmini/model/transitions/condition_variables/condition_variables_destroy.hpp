#pragma once

#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transitions/mutex/mutex_lock.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_init.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_wait.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_signal.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variable_enqueue_thread.hpp"

namespace model {
namespace transitions {

struct condition_variable_destroy : public model::transition {
 private:
  const state::objid_t cond_id;

 public:
  condition_variable_destroy(runner_id_t executor, state::objid_t cond_id)
    : transition(executor), cond_id(cond_id) {}
  ~condition_variable_destroy() = default;

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
    
    // Check if there are still threads waiting on this condition variable
    if (cv->has_waiters()) {
      return status::undefined; // Error: destroying condition variable with waiters
    }
    
    // Mark condition variable as destroyed
    s.add_state_for_obj(cond_id, new condition_variable(condition_variable::cv_destroyed, 
        executor, 
        nullptr,  // Clear mutex association
        0));
    return status::exists;
  }
  state::objid_t get_id() const { return this->cond_id; }
  std::string to_string() const override {
    return "pthread_cond_destroy(condition_variable:" + std::to_string(cond_id) + ")";
  }
  // MARK: Model checking functions
  bool coenabled_with(const condition_variable_wait* cw) const {
    return this->cond_id != cw->get_id(); // Can't destroy CV if threads waiting
  }

  bool coenabled_with(const condition_variable_signal* cs) const {
    return this->cond_id != cs->get_id(); // Can't signal and destroy same CV
  }

  bool depends(const condition_variable_wait* cw) const {
    return this->cond_id == cw->get_id(); // Dependent with wait on same CV
  }

  bool depends(const condition_variable_signal* cs) const {
    return this->cond_id == cs->get_id(); // Dependent with signal on same CV
  }

};
} // namespace transitions
} // namespace model
