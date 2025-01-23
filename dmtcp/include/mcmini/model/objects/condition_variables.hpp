#pragma once

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"
#include "mcmini/misc/cond/cond_var_arbitrary_policy.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include <string>

namespace model {
namespace objects {

struct condition_variable : public model::visible_object_state {
 public:
  /* The four possible states for a condition variable */
  enum state { 
        cv_uninitialized, 
        cv_initialized, 
        cv_waiting, 
        cv_signalled, 
        cv_transitional 
        };

 private:
  state current_state = state::cv_uninitialized;
   int waiting_count = 0;
  bool hadwaiters;
  unsigned int numRemainingSpuriousWakeups = 0;
  runner_id_t running_thread;
  pthread_mutex_t* associated_mutex;
  std::unique_ptr<ConditionVariablePolicy> policy = std::make_unique<ConditionVariableArbitraryPolicy>();
  

 public:
  condition_variable() = default;
  ~condition_variable() = default;
  condition_variable(const condition_variable &) = default;
  condition_variable(state s) : current_state(s) {}
  condition_variable(state s, std::unique_ptr<ConditionVariablePolicy> p) : current_state(s), policy(std::move(p)) {}
  condition_variable(state s, int count) : current_state(s) {}
  condition_variable(state s, runner_id_t tid, pthread_mutex_t* mutex) 
    : current_state(s), running_thread(tid), associated_mutex(mutex) {
    if (this->policy) {
        this->policy->add_waiter(tid);
        waiting_count++; // Increment the count of waiting threads if needed
    }
}
  // ---- State Observation --- //
  bool operator==(const condition_variable &other) const {
  return this->current_state == other.current_state;
  }
  bool operator!=(const condition_variable &other) const {
  return this->current_state != other.current_state;
  }
  bool is_initialized() const { return this->current_state == cv_initialized ; }
  bool is_waiting() const { return this->current_state == cv_waiting ; }
  bool is_signalled() const { return this->current_state == cv_signalled ; }
  bool is_uninitialized() const { return this->current_state == cv_uninitialized ;}
  bool is_transitional() const { return this->current_state == cv_transitional;}

  pthread_mutex_t* get_mutex() const {return this->associated_mutex;}

  bool has_waiters() const {return this->policy->has_waiters();}
  bool remove_waiter(runner_id_t tid) {
        this->policy->wake_thread(tid);
        // If there are any spurious wake ups allowed,
        // we always allow the thread to wake up
        // due to a spurious wake up and decrement the
        // number of future such wakeups allowed
        if (this->numRemainingSpuriousWakeups > 0) {
            this->numRemainingSpuriousWakeups--;
        }
  }
  void add_waiter(runner_id_t tid) {
    this->policy->add_waiter(tid);
  }
  bool waiter_can_exit(runner_id_t tid) {
    return this->numRemainingSpuriousWakeups > 0 || 
    this->policy->thread_can_exit(tid);
  }

  void send_signal_message() {
        this->policy->receive_signal_message();
  }

  void send_broadcast_message() {
        this->policy->receive_broadcast_message();
  }
     
  std::unique_ptr<visible_object_state> clone() const override {
    return extensions::make_unique<condition_variable>(*this);
  }
  std::string to_string() const override {
    return "condition_variable(state: " + std::to_string(current_state);  
    }
};
}  // namespace objects
}  // namespace model
