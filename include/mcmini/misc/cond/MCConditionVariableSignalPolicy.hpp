#ifndef INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLESIGNALPOLICY_HPP
#define INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLESIGNALPOLICY_HPP

#include "mcmini/MCShared.h"
#include "mcmini/misc/cond/MCWakeGroup.hpp"
#include <memory>

namespace mcmini {

struct ConditionVariableSignalPolicy {

  virtual std::unique_ptr<ConditionVariableSignalPolicy>
  clone() const = 0;

  virtual void addWaiter(tid_t tid)    = 0;
  virtual void removeWaiter(tid_t tid) = 0;

  /**
   * @brief Simulate sending a signal to the condition variable
   * implementing this policy
   *
   * When a condition variable is signaled, it allows a single thread
   * to awaken from the condition variable. Which thread is awoken
   * depends on the underlying behavior of the condition variable as
   * described by the condition variable's wakeup policy. When signal
   * the policy is made aware of a signal sent to a condition
   * variable, you should update any internal state to properly
   * reflect the fact that the condition variable has been signaled
   *
   * @note for condition variables that support spurious wake-ups, it
   * is possible for a thread to wake up without the condition
   * variable first receiving a signal/broadcast
   */
  virtual WakeGroup receiveSignalMessage() = 0;

  /**
   * @brief Simulate sending a broadcast message to the condition
   * variable implementing this policy
   *
   * When a condition variable is sent a broadcast message, it wakes
   * all sleeping threads. The order in which threads actually exit
   * the condition variable depends on the underlying behavior of the
   * condition variable as described by the condition variable's
   * wakeup policy. When the signal policy is made aware of a
   * broadcast sent to a condition variable, you should update any
   * internal state to properly reflect the fact that the condition
   * variable has been sent a broadcast
   *
   * @note for condition variables that support spurious wake-ups, it
   * is possible for a thread to wake up without the condition
   * variable first receiving a broadcast
   */
  virtual WakeGroup receiveBroadcastMessage() = 0;
};

} // namespace mcmini
#endif // INCLUDE_MCMINI_MISC_COND_MCCONDITIONVARIABLESIGNALPOLICY_HPP
