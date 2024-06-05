#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct mutex_init : public model::transition {
 private:
  const state::objid_t mutex_id; /* The mutex this transition initializes */

 public:
  mutex_init(runner_id_t executor, state::objid_t mutex_id)
      : transition(executor), mutex_id(mutex_id) {}
  ~mutex_init() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    s.add_state_for_obj(mutex_id, new mutex(mutex::unlocked));
    return status::exists;
  }
  state::objid_t get_id() const { return this->mutex_id; }
  std::string to_string() const override {
    return "pthread_mutex_init(mutex:" + std::to_string(mutex_id) + ")";
  }
};
}  // namespace transitions
}  // namespace model
