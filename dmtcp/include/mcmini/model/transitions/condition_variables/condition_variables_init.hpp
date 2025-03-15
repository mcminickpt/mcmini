#pragma once

#include "mcmini/model/objects/condition_variables.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct condition_variable_init : public model::transition{
 private:
  const state::objid_t condition_variable_id; /* The condition variable this transition initializes */

 public:
  condition_variable_init(runner_id_t executor, state::objid_t condition_variable_id)
      : transition(executor), condition_variable_id(condition_variable_id) {}
  ~condition_variable_init() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    s.add_state_for_obj(condition_variable_id, new condition_variable(condition_variable::cv_initialized));
    return status::exists;
  }
  state::objid_t get_id() const { return this->condition_variable_id; }
  std::string to_string() const override {
    return "pthread_cond_init(condition_variable:" + std::to_string(condition_variable_id) + ")";
  }
};
}  // namespace transitions
}  // namespace model
