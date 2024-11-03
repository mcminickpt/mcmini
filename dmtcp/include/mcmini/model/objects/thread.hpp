#pragma once

#include <string>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace model {
namespace objects {

struct thread : public model::runner_state {
 public:
  /* The four possible states for a mutex */
  enum state { running, exited, killed, embryo };

 private:
  state current_state = state::embryo;

 public:
  thread() = default;
  ~thread() = default;
  thread(const thread &) = default;
  thread(state state) : current_state(state) {}
  // ---- State Observation --- //
  bool operator==(const thread &other) const {
    return this->current_state == other.current_state;
  }
  bool operator!=(const thread &other) const {
    return this->current_state != other.current_state;
  }
  bool is_embryo() const { return this->current_state == embryo; }
  bool is_running() const { return this->current_state == running; }
  bool has_exited() const { return this->current_state == exited; }
  bool is_killed() const { return this->current_state == killed; }
  bool is_active() const override { return this->is_running(); }
  std::unique_ptr<visible_object_state> clone() const override {
    return extensions::make_unique<thread>(*this);
  }
  std::string to_string() const override {
    return "thread(" + std::to_string(current_state) + ")";
  }
};
}  // namespace objects
}  // namespace model
