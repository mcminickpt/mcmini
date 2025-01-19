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

    if (cv->is_initialized() && m->is_locked()) {
      // If the condition variable is initialized or signalled, we move to the transitional state
      s.add_state_for_obj(cond_id, new condition_variable(condition_variable::cv_transitional));
    } else {
      s.add_state_for_obj(mutex_id, new mutex(mutex::locked));
      
    }
    s.add_state_for_obj(cond_id, new condition_variable(condition_variable::cv_waiting));

    return status::exists;
  }
  state::objid_t get_id() const { return this->cond_id; }
  std::string to_string() const override {
    return "pthread_cond_wait(cond:" + std::to_string(cond_id) + ", mutex:" + std::to_string(mutex_id) + ")";
  }

  // MARK: Model checking functions
    bool depends(const condition_variable_init* ci) const {
        return this->cond_id == ci->get_id();
    }
    bool depends(const mutex_lock* ml) const {
        return this->mutex_id == ml->get_id();
    }
};
} // namespace transitions
} // namespace model
