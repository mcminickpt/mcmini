#pragma once

#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct process_abort : public model::transition {
 public:
  process_abort(state::runner_id_t executor) : transition(executor) {}
  ~process_abort() = default;

  status modify(model::mutable_state& s) const override {
    // We ensure that aborting is never enabled. This ensures that it will never
    // be explored by any model checking algorithm
    return status::disabled;
  }

  std::string to_string() const override { return "abort(2) (syscall)"; }
};

}  // namespace transitions
}  // namespace model
