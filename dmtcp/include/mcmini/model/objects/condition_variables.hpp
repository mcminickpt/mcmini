#pragma once

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"
#include "mcmini/misc/cond/cond_var_arbitrary_policy.hpp"
#include "mcmini/model/objects/mutex.hpp"
#include "mcmini/Thread_queue.h"
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
        cv_transitional,
        cv_destroyed
        };

 private:
  state current_state = state::cv_uninitialized;
   
  bool hadwaiters;
  mutable unsigned int numRemainingSpuriousWakeups = 0;
  runner_id_t running_thread;
  pthread_mutex_t* associated_mutex;
  int waiting_count = 0;
  ConditionVariablePolicy* policy = new ConditionVariableArbitraryPolicy();  

 public:
  condition_variable() = default;
  ~condition_variable() = default;
  condition_variable(const condition_variable &) = default;
  condition_variable(state s) : current_state(s) {}
  condition_variable(state s, ConditionVariablePolicy* p) : current_state(s), policy(p) {}
  condition_variable(state s, int count) : current_state(s) {}
  condition_variable(state s, runner_id_t tid, pthread_mutex_t* mutex, int count) 
    : current_state(s), running_thread(tid), associated_mutex(mutex), waiting_count(count){}

    condition_variable(state s, runner_id_t tid, pthread_mutex_t* mutex, int count, 
      const std::vector<std::pair<runner_id_t, condition_variable_status>>& thread_states) 
    : current_state(s), running_thread(tid), associated_mutex(mutex), waiting_count(count) {
      // Initialize the policy according to the states of the threads in waiting queue
      for (const auto& thread_with_state : thread_states) {
        if (thread_with_state.second == CV_TRANSITIONAL || 
            thread_with_state.second == CV_WAITING) {
          this->policy->add_waiter_with_state(thread_with_state.first,thread_with_state.second);
        }
      }
      std::vector<runner_id_t> signaled_threads;
      for (const auto& thread_with_state : thread_states) {
        if (thread_with_state.second == CV_SIGNALLED) {
          signaled_threads.push_back(thread_with_state.first);
        }
      }
      if(!signaled_threads.empty()) {
        // If there are any threads that have been signaled, we should
        // add them to the wake groups in the policy.
        this->policy->add_to_wake_groups(signaled_threads);
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
  bool is_destroyed() const { return this->current_state == cv_destroyed;}

  ConditionVariablePolicy* get_policy() const {return this->policy;}

  void set_associated_mutex(pthread_mutex_t* mutex) {
    this->associated_mutex = mutex;
  }


  pthread_mutex_t* get_mutex() const {return this->associated_mutex;}

  bool has_waiters() const {return this->policy->has_waiters();}
  void remove_waiter(runner_id_t tid) const  {
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
  bool waiter_can_exit(runner_id_t tid) const{
    return this->numRemainingSpuriousWakeups > 0 || 
    this->policy->thread_can_exit(tid);
  }

  void send_signal_message() const{
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
