#pragma once

#include "mcmini/defines.h"
#include "cond_var_wakegroup.hpp"


#include <exception>
#include <memory>
#include <deque>


/**
 * @brief A state machine determining how threads waiting on a
 * condition variable are selected for being woken
 *
 * A `ConditionVariablePolicy` encapsulates a particular behavior a
 * condition variable could exhibit. Abstractly, condition variables
 * manage sleeping threads and change each thread's eligibility for
 * waking up and "escaping" the condition variable as the condition
 * variable receives signals and broadcast messages. A policy
 * determines, for each sleeping thread and for each sequence of
 * thread additions and signals/broadcasts to a condition variable,
 * whether a given thread is allowed to consume a signal. A policy
 * emulates the true runtime behavior of threads interacting with
 * condition variables, viz. the atomic consumption of a
 * signal/broadcast message
 */

class ConditionVariablePolicy {
public:
  /**
   * @brief Simulate sending a signal to the condition variable
   * implementing this policy
   *
   * When signaling, it allows a single thread to awaken from the
   * condition variable. Which thread is woken depends on the
   * underlying behavior of the condition variable as described by the
   * condition variable's wakeup policy. When signal the policy is
   * made aware of a signal sent to a condition variable, any internal
   * state should be updated to properly reflect the fact that the
   * condition variable has been signaled.
   *
   * @note for condition variables that support spurious wake-ups, it
   * is possible for a thread to wake up without the condition
   * variable first receiving a signal/broadcast.
   */
  virtual void receive_signal_message() = 0;

  /**
   * @brief Simulate sending a broadcast message to the condition
   * variable implementing this policy
   *
   * When a condition variable is sent a broadcast message, it wakes
   * all sleeping threads. The order in which threads actually exit
   * the condition variable depends on the underlying behavior of the
   * condition variable as described by the condition variable's
   * wakeup policy. When the signal policy is made aware of a
   * broadcast sent to a condition variable, any internal state should
   * be updated to properly reflect the fact that the condition
   * variable has been signaled.
   *
   * @note for condition variables that support spurious wake-ups, it
   * is possible for a thread to wake up without the condition
   * variable first receiving a broadcast.
   */
  virtual void receive_broadcast_message() = 0;

  /**
   * @brief Whether or not the given thread can currently escape a
   * condition variable implementing this policy
   *
   * The conditions which may affect whether or not a thread is
   * allowed to exit from the condition variable are arbitrary.
   * Each policy decides, based on any internal state managed by the
   * policy tracking changes to the state of the condition variable,
   * under what conditions a thread is enabled
   *
   * @note whether or not a thread is enabled can change based on the
   * number of and/or relative ordering of signals/broadcasts sent to
   * the condition variable.
   *
   * @param tid the identity of the thread to check
   * @return true if the thread can exit from the condition variable
   * @return false if the thread is not allowed to wake from the
   * condition variable
   */
  virtual bool thread_can_exit(runner_id_t tid) const = 0;

  /**
   * @brief Removes the given thread from the management
   * of the condition variable
   *
   * After calling `wakeThread()` on the policy, the supplied thread
   * is no longer managed by the policy and all references to the
   * thread are removed from the condition variable. Conceptually,
   * the thread has now "woken" and has "escaped" the condition
   * variable
   *
   * @note after the given thread has woken, the criterion for
   * whether a thread is allowed to exit may have changed. Thus the
   * return type
   *
   * @param tid the thread to wake up and allow to escape from the
   * condition variable
   *
   * @throws invalid_thread_wakeup_exception if the thread is now
   * allowed to exit the condition variable
   */
  virtual void wake_thread(runner_id_t tid) = 0;

  /**
   * @brief Mark a new thread as sleeping on the condition variable
   *
   * @param tid the thread to mark as sleeping on the condition
   * variable
   */
  virtual void add_waiter(runner_id_t tid) = 0;

  virtual bool has_waiters() const = 0;

  virtual ConditionVariablePolicy* clone() const = 0;

  virtual ~ConditionVariablePolicy() = default;

  virtual std::deque<runner_id_t> return_wait_queue() const = 0;
  virtual std::vector<WakeGroup> return_wake_groups() const = 0;

  struct invalid_thread_addition : public std::exception {
    const char *
    what() const noexcept override
    {
      return "Attempted to put to sleep a thread already sleeping on "
             "a condition variable";
    }
  };

  struct invalid_thread_wakeup_exception : public std::exception {
    const char *
    what() const noexcept override
    {
      return "Attempted to wake an ineligible thread on a condition "
             "variable";
    }
  };
};

