#pragma once

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace model::objects {

struct thread : public model::visible_object_state {
 public:
  /* The four possible states for a mutex */
  enum state_type { embryo, running, exited, killed };

 private:
  state_type current_state = state_type::embryo;

 public:
  thread() = default;
  ~thread() = default;
  thread(const thread &) = default;
  thread(state_type state) : current_state(state) {}
  static std::unique_ptr<thread> make(state_type state) {
    return extensions::make_unique<thread>(state);
  }
  static std::unique_ptr<thread> make() { return thread::make(embryo); }

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

  std::unique_ptr<visible_object_state> clone() const override {
    return extensions::make_unique<thread>(*this);
  }
};

}  // namespace model::objects
