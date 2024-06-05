#pragma once

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace model {
namespace objects {

struct mutex : public model::visible_object_state {
 public:
  /* The four possible states for a mutex */
  enum state_type { uninitialized, unlocked, locked, destroyed };

 private:
  state_type current_state = state_type::uninitialized;

 public:
  mutex() = default;
  ~mutex() = default;
  mutex(const mutex &) = default;
  mutex(state_type state) : current_state(state) {}
  // ---- State Observation --- //
  bool operator==(const mutex &other) const {
    return this->current_state == other.current_state;
  }
  bool operator!=(const mutex &other) const {
    return this->current_state != other.current_state;
  }
  bool is_locked() const { return this->current_state == locked; }
  bool is_unlocked() const { return this->current_state == unlocked; }
  bool is_destroyed() const { return this->current_state == destroyed; }
  bool is_initialized() const { return this->current_state != uninitialized; }

  std::unique_ptr<visible_object_state> clone() const override {
    return extensions::make_unique<mutex>(*this);
  }
  std::string to_string() const override {
    return "mutex(" + std::to_string(current_state) + ")";
  }
};
}  // namespace objects
}  // namespace model
