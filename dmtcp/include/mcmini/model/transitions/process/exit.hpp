#pragma once

#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct process_exit : public model::transition {
 public:
  process_exit(state::runner_id_t executor) : transition(executor) {}
  ~process_exit() = default;

  status modify(model::mutable_state& s) const override {
    // We ensure that exiting is never enabled. This ensures that it will never
    // be explored by any model checking algorithm
    return status::disabled;
  }

  std::string to_string() const override { return "exit(2) (syscall)"; }
};

}  // namespace transitions
}  // namespace model
