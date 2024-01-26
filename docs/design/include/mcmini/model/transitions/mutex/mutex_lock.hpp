#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::model::transitions {

struct mutex_lock : public mcmini::model::transition {
 private:
  state::objid_t mutex_id; /* The mutex this transition initializes */

 public:
  mutex_lock(state::objid_t mutex_id) : mutex_id(mutex_id) {}
  ~mutex_lock() = default;

  status modify(mcmini::model::mutable_state& s) const override {
    using namespace mcmini::model::objects;

    // A `mutex_lock` cannot be applied to a mutex already locked.
    const mutex* ms = s.get_state_of_object<mutex>(mutex_id);
    if (ms->is_locked()) {
      return status::disabled;
    }
    s.add_state_for(mutex_id, mutex::make(mutex::locked));
    return status::exists;
  }

  std::string to_string() const override {
    return "mutex_lock(" + std::to_string(mutex_id) + ")";
  }
};

}  // namespace mcmini::model::transitions