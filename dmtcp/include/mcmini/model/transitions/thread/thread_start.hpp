#pragma once

#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct thread_start : public model::transition {
 public:
  thread_start(state::runner_id_t executor) : transition(executor) {}
  ~thread_start() = default;

  status modify(model::mutable_state& s) const override {
    // No modification necessary: we simply move into the next state
    using namespace model::objects;
    auto* thread_state = s.get_state_of_runner<thread>(executor);
    return thread_state->is_embryo() ? status::disabled : status::exists;
  }

  std::string to_string() const override { return "starts"; }
};

}  // namespace transitions
}  // namespace model
