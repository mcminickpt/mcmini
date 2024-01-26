#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::model::transitions {

struct mutex_init : public mcmini::model::transition {
 private:
  state::objid_t mutex_id; /* The mutex this transition initializes */

 public:
  mutex_init(state::objid_t mutex_id) : mutex_id(mutex_id) {}
  ~mutex_init() = default;

  status modify(mcmini::model::mutable_state& s) const override {
    using namespace mcmini::model::objects;
    s.add_state_for(mutex_id, mutex::make(mutex::unlocked));
    return status::exists;
  }

  std::string to_string() const override {
    return "mutex_init(" + std::to_string(mutex_id) + ")";
  }
};

}  // namespace mcmini::model::transitions