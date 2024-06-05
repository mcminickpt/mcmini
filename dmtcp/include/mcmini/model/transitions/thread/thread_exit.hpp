#pragma once

#include "mcmini/defines.h"
#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct thread_exit : public model::transition {
 public:
  thread_exit(state::runner_id_t executor) : transition(executor) {}
  ~thread_exit() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    auto* thread_state = s.get_state_of_runner<thread>(executor);
    if (!thread_state->is_running() || executor == RID_MAIN_THREAD) {
      return status::disabled;
    }
    s.add_state_for_runner(executor, new thread(thread::exited));
    return status::exists;
  }

  std::string to_string() const override { return "exits"; }
};

}  // namespace transitions
}  // namespace model
