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

  public:
   condition_variable_signal(runner_id_t executor, state::objid_t cond_id)
       : transition(executor), cond_id(cond_id) {}
  ~condition_variable_signal() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;

    // Retrive the state of the condition variable
    const condition_variable* cv = s.get_state_of_object<condition_variable>(cond_id);

    // Ensure that the condition variable is in the waiting state
    if (!cv->is_waiting()) {
      return status::disabled;
    }

    // Decrement the waiting count of the condition variable
    int current_waiting_count = cv->get_waiting_count() - 1;

    // If there are still threads waiting , we stay in the waiting state
    // Otherwise, we move to the signalled state
    condition_variable::state new_state = current_waiting_count > 0 ? condition_variable::cv_waiting : condition_variable::cv_signalled;

    s.add_state_for_obj(cond_id, new condition_variable(new_state, current_waiting_count));
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
