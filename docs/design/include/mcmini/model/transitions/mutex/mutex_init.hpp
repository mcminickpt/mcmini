#pragma once

#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/real_world/remote_address.hpp"

namespace mcmini::model::transitions {

struct mutex_init : public mcmini::model::transition {
 private:
  state::objid_t mutex; /* The mutex this transition initializes */

 public:
  mutex_init(state::objid_t mutex) : mutex(mutex) {}
  ~mutex_init() = default;

  status modify(mcmini::model::mutable_state& s) const override {
    auto m = s.get_state_of_object<model::objects::mutex_state>(0);
    s.add_state_for(mutex, mcmini::model::objects::mutex_state::make(
                               mcmini::model::objects::mutex_state::unlocked));
    return status::exists;
  }

  std::unique_ptr<transition> deserialize_from_wrapper_contents(
      std::istream& is, model_to_system_map& map) const override {
    // mcmini::real_world::remote_address<pthread_mutex_t> a;

    // is >> a;

    return mcmini::extensions::make_unique<mutex_init>(*this);
  }

  std::string to_string() const override { return "mutex_init()"; }
};

}  // namespace mcmini::model::transitions