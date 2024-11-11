#pragma once

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"
#include <string>

namespace model {
namespace objects {

struct condition_variable : public model::visible_object_state {
 public:
  /* The four possible states for a condition variable */
  enum state { cv_uninitialized, cv_initialized, cv_waiting, cv_signalled, cv_transitional };

 private:
  state current_state = state::cv_uninitialized;
  int waiting_count = 0;

 public:
  condition_variable() = default;
  ~condition_variable() = default;
  condition_variable(const condition_variable &) = default;
  condition_variable(state s) : current_state(s) {}
  condition_variable(state s, int count) : current_state(s), waiting_count(count) {}
  condition_variable(state s, pthread_t waiting_thread, pthread_mutex_t *mutex, int count) : current_state(s), waiting_count(count) {}
  int get_waiting_count() const { return waiting_count; }
  // ---- State Observation --- //
  bool operator==(const condition_variable &other) const {
  return this->current_state == other.current_state &&
          this->waiting_count == other.waiting_count;
  }
  bool operator!=(const condition_variable &other) const {
  return this->current_state != other.current_state ||
          this->waiting_count != other.waiting_count;
  }
  bool is_initialized() const { return this->current_state == cv_initialized && this->waiting_count == 0; }
  bool is_waiting() const { return this->current_state == cv_waiting && this->waiting_count > 0; }
  bool is_signalled() const { return this->current_state == cv_signalled && this->waiting_count >=0; }
  bool is_uninitialized() const { return this->current_state == cv_uninitialized && this->waiting_count == 0; }
  bool is_transitional() const { return this->current_state == cv_transitional;}
     
  std::unique_ptr<visible_object_state> clone() const override {
    return extensions::make_unique<condition_variable>(*this);
  }
  std::string to_string() const override {
    return "condition_variable(state: " + std::to_string(current_state) +
           ", waiting_count: " + std::to_string(waiting_count) + ")";  
    }
};
}  // namespace objects
}  // namespace model
