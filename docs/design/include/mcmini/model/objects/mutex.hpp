#pragma once

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace mcmini::model::objects {

struct mutex_state : public mcmini::model::visible_object_state {
 public:
  /* The four possible states for a mutex */
  enum state_type { uninitialized, unlocked, locked, destroyed };

 private:
  state_type current_state = state_type::uninitialized;

 public:
  mutex_state() = default;
  ~mutex_state() = default;
  mutex_state(const mutex_state &) = default;
  mutex_state(state_type state) : current_state(state) {}
  static std::unique_ptr<mutex_state> make(state_type state) {
    return mcmini::extensions::make_unique<mutex_state>(state);
  }

  // ---- State Observation --- //
  bool operator==(const mutex_state &other) const {
    return this->current_state == other.current_state;
  }
  bool operator!=(const mutex_state &other) const {
    return this->current_state != other.current_state;
  }
  bool is_locked() const { return this->current_state == locked; }
  bool is_unlocked() const { return this->current_state == unlocked; }
  bool is_destroyed() const { return this->current_state == destroyed; }
  bool is_initialized() const { return this->current_state != uninitialized; }

  std::unique_ptr<visible_object_state> clone() const override {
    return mcmini::extensions::make_unique<mutex_state>(*this);
  }
};

}  // namespace mcmini::model::objects
