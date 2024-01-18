#pragma once

#include "mcmini/model/objects/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::model::transitions {

struct thread_start : public mcmini::model::transition {
 private:
  state::objid_t thread_id; /* The mutex this transition initializes */

 public:
  thread_start(state::objid_t thread_id) : thread_id(thread_id) {}
  ~thread_start() = default;

  status modify(mcmini::model::mutable_state& s) const override {
    using namespace mcmini::model::objects;
    s.add_state_for(thread_id, thread::make(thread::running));
    return status::exists;
  }

  std::string to_string() const override {
    return "thread_start(" + std::to_string(thread_id) + ")";
  }
};

}  // namespace mcmini::model::transitions