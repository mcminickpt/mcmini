#pragma once

#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transitions/mutex/mutex_lock.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/condition_variables/condition_variables_init.hpp"

namespace model {
namespace transitions {

struct condition_variable_enqueue_thread : public model::transition{
 private:
  const state::objid_t cond_id;
  const state::objid_t mutex_id;
 public:
  condition_variable_enqueue_thread(runner_id_t executor, state::objid_t cond_id, state::objid_t mutex_id)
      : transition(executor), cond_id(cond_id), mutex_id(mutex_id) {}
  ~condition_variable_enqueue_thread() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;

    const condition_variable* cv = s.get_state_of_object<condition_variable>(cond_id);
    const mutex* m = s.get_state_of_object<mutex>(mutex_id);

    if (m->is_unlocked()) {
      return status::disabled;
    }

    if(!m->is_locked_by(executor)) {
      return status::disabled;
    }

    condition_variable_status current_state = cv->get_policy()->get_thread_cv_state(executor);
    if (current_state == CV_PREWAITING) {
    // Thread not fully in wait state - update to WAITING before proceeding
      cv->get_policy()->update_thread_cv_state(executor, CV_WAITING);
    }

    cv->get_policy()->add_waiter_with_state(executor, CV_WAITING);
    const int new_waiting_count = cv->get_policy()->return_wait_queue().size();
    
    s.add_state_for_obj(cond_id, new condition_variable(condition_variable::cv_waiting, executor, m->get_location(), new_waiting_count));
    s.add_state_for_obj(mutex_id, new mutex(mutex::unlocked));
    return status::exists;
  }
  state::objid_t get_id() const { return this->cond_id; }
  state::objid_t get_mutex_id() const { return this->mutex_id; }  
  std::string to_string() const override {
    return "pthread_cond_wait(cond:" + std::to_string(cond_id) + ", mutex:" + std::to_string(mutex_id) + "(awake -> asleep))";
  }

  // MARK: Model checking functions
  bool depends (const condition_variable_init* ci) const {
    return this->cond_id == ci->get_id();
  }

  bool depends (const condition_variable_wait* cw) const {
    return this->cond_id == cw->get_id();
  }

  bool depends (const condition_variable_signal* cs) const {
    return this->cond_id == cs->get_id();
  }

  bool depends (const mutex_lock* ml) const {
    return this->mutex_id == ml->get_id();
  }

  bool depends (const mutex_unlock* mu) const {
    return this->mutex_id == mu->get_id();
  }

  bool coenabled_with (const condition_variable_enqueue_thread* cwt ) const {
    return this->cond_id != cwt->get_id();
  }

  bool coenabled_with (const mutex_lock* ml) const {
    return this->mutex_id != ml->get_id();
  }
  
  bool coenabled_with (const mutex_unlock* mu) const {
    return this->mutex_id != mu->get_id();
  }
};
}
}