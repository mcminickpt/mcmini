#pragma once

#include "mcmini/coordinator/model_to_system_map.hpp"
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
    using namespace mcmini::model::objects;
    s.add_state_for(mutex, mutex_state::make(mutex_state::unlocked));
    return status::exists;
  }

  std::string to_string() const override { return "mutex_init()"; }

  // Deserialization routine (could [and should] be moved outside the model)

  static std::unique_ptr<transition> from_wrapper_contents(
      std::istream& is, model_to_system_map& map) {
    using namespace mcmini::model::objects;
    void* mutex_addr;
    is >> mutex_addr;
    auto mutex_id = map.observe_remote_process_handle(
        mutex_addr, mutex_state::make(mutex_state::uninitialized));
    return mcmini::extensions::make_unique<mutex_init>(mutex_id);
  }
};

}  // namespace mcmini::model::transitions