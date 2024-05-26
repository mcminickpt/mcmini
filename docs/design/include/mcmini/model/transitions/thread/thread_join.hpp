#pragma once

#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct thread_join : public model::transition {
 private:
  state::runner_id_t target;

 public:
  thread_join(state::runner_id_t executor, state::runner_id_t target)
      : transition(executor), target(target) {}
  ~thread_join() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    auto* target_state = s.get_state_of_runner<thread>(target);
    return target_state->has_exited() ? status::exists : status::disabled;
  }
  state::runner_id_t get_target() const { return target; }
  std::string to_string() const override {
    return "pthread_join(thread: " + std::to_string(target) + ")";
  }

  // MARK: Dependencies
  bool depends(const model::transition* t) const {
    return this->target == t->get_executor();
  }
  bool coenabled_with(const model::transition* t) const {
    return this->target != t->get_executor();
  }
};

}  // namespace transitions
}  // namespace model
