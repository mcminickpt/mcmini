#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::model::transitions {

struct mutex_init : public mcmini::model::transition {
 private:
  state::objid_t mutex; /* The mutex this transition initializes */

 public:
  mutex_init(state::objid_t mutex) : mutex(mutex) {}
  ~mutex_init() = default;
  status modify(mcmini::model::mutable_state& s) const override {
    s.add_state_for(mutex, mcmini::model::objects::mutex_state::make(
                               mcmini::model::objects::mutex_state::unlocked));
    return status::exists;
  }

  std::unique_ptr<transition> deserialize_from_wrapper_contents(
      std::istream&, model_to_system_map&) const override {
    return mcmini::extensions::make_unique<mutex_init>(*this);
  }

  std::string to_string() const override { return "mutex_init()"; }
};

}  // namespace mcmini::model::transitions