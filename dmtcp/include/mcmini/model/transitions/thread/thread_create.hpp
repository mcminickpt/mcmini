#pragma once

#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct thread_create : public model::transition {
 private:
  state::runner_id_t target;

 public:
  thread_create(state::runner_id_t executor, state::runner_id_t target)
      : transition(executor), target(target) {}
  ~thread_create() = default;

  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    s.add_state_for_runner(target, new thread(thread::running));
    return status::exists;
  }
  state::runner_id_t get_target() const { return target; }
  std::string to_string() const override {
    return "pthread_create(thread: " + std::to_string(target) + ")";
  }

  // MARK: DPOR Methods
  bool depends(const model::transition* t) const {
    return this->target == t->get_executor();
  }
  bool coenabled_with(const model::transition *t) const {
    return this->target != t->get_executor();
  }
};

}  // namespace transitions
}  // namespace model
