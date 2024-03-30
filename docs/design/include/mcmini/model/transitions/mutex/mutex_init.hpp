#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct mutex_init : public model::transition {
 private:
  state::objid_t mutex_id; /* The mutex this transition initializes */

 public:
  mutex_init(state::objid_t mutex_id) : mutex_id(mutex_id) {}
  ~mutex_init() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    s.add_state_for(mutex_id, mutex::make(mutex::unlocked));
    return status::exists;
  }

  std::string to_string() const override {
    return "mutex_init(" + std::to_string(mutex_id) + ")";
  }
};
}  // namespace transitions
}  // namespace model