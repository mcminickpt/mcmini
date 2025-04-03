#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct mutex_unlock : public model::transition {
 private:
  const state::objid_t mutex_id; /* The mutex this transition unlocks */

 public:
  mutex_unlock(runner_id_t executor, state::objid_t mutex_id)
      : transition(executor), mutex_id(mutex_id) {}
  ~mutex_unlock() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    // TODO: If the mutex already unlocked, this would be erroneous program
    // behavior. We should distinguish between this and other cases.
    const mutex* ms = s.get_state_of_object<mutex>(mutex_id);
    // Validate ownership
    if (!ms->is_locked_by(this->executor)) {
      return status::disabled;
    }

    s.add_state_for_obj(mutex_id, new mutex(mutex::unlocked, ms->get_location(), 0));
    return status::exists;
  }
  state::objid_t get_id() const { return this->mutex_id; }
  std::string to_string() const override {
    return "pthread_mutex_unlock(mutex:" + std::to_string(mutex_id) + ")";
  }
};
}  // namespace transitions
}  // namespace model
