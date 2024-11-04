#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/mutex/mutex_unlock.hpp"
#include "mcmini/model/transitions/mutex/mutex_init.hpp"

namespace model {
namespace transitions {

struct mutex_lock : public model::transition {
 private:
  const state::objid_t mutex_id; /* The mutex this transition locks */

 public:
  mutex_lock(runner_id_t executor, state::objid_t mutex_id)
      : transition(executor), mutex_id(mutex_id) {}
  ~mutex_lock() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;

    // A `mutex_lock` cannot be applied to a mutex already locked.
    const mutex* ms = s.get_state_of_object<mutex>(mutex_id);
    if (ms->is_locked()) {
      return status::disabled;
    }
    s.add_state_for_obj(mutex_id, new mutex(mutex::locked, ms->get_location(), this->executor));
    return status::exists;
  }
  state::objid_t get_id() const { return this->mutex_id; }
  std::string to_string() const override {
    return "pthread_mutex_lock(mutex:" + std::to_string(mutex_id) + ")";
  }

  // MARK: Model checking functions
  bool depends(const mutex_init* mi) const {
    return this->mutex_id == mi->get_id();
  }
  bool depends(const mutex_lock* ml) const {
    return this->mutex_id == ml->get_id();
  }
  bool coenabled_with(const mutex_unlock* mu) const {
    return this->mutex_id != mu->get_id();
  }
};
}  // namespace transitions
}  // namespace model
