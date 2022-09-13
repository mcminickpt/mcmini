#ifndef INCLUDE_MCMINI_MISC_MCCONDITIONVARIABLEWAKEUPPOLICY_HPP
#define INCLUDE_MCMINI_MISC_MCCONDITIONVARIABLEWAKEUPPOLICY_HPP

#include "mcmini/MCShared.h"
#include "mcmini/misc/cond/MCWakeGroup.hpp"
#include <exception>
#include <memory>

namespace mcmini {

/**
 * @brief A state machine determining how threads waiting on a
 * condition variable are selected for being awoken
 *
 * There are numerous ways that threads sleeping on a condition
 * variable can be awoken. A `ConditionVariablePolicy` encapsulates
 * a particular behavior a condition variable could exhibit.
 * Abstractly, condition variables manage sleeping threads and change
 * each thread's elligibility for waking up and "escaping" the
 * condition variable as the condition variable is signaled.
 *
 * A policy determines, for each sleeping thread and for each sequence
 * of thread additions and signals/broadcasts to a condition variable,
 * whether a given thread is allowed to wake up. Wake-up policies
 * emulate the true runtime behavior of threads interacting with
 * condition variables.
 *
 * Threads are queued by the wakeup policy into *wake groups*. A wake
 * group is a set of threads elligible to escape the condition
 * variable by virtue of having been sleeping on the condition
 * variable at the time a signal/broadcast was sent to the condition
 * variable. See the documentation for `mcmini::WakeGroup` for more
 * details
 */
class ConditionVariableWakeupPolicy {
public:

  virtual std::unique_ptr<ConditionVariableWakeupPolicy>
  clone() const = 0;

  /**
   * @brief Pushes the given wakegroup to the end of the wakeup queue
   * managed by the policy
   *
   * @invariant if wake groups are created via a (well-formed)
   * ConditionVariableSignalPolicy, then the wake groups following and
   * including the first wake group to contain thread `i` will all
   * contain thread `i`. This can be deduced by noting that a thread
   * which was elligible to wake-up at a prior signal-point will
   * remain elligible to wake up if it has not already done so by the
   * time a subsequent signal/broadcast is sent to the condition
   * variable.
   *
   * @param group the group to push to the end of the policy
   */
  virtual void pushWakeupGroup(const WakeGroup &group) = 0;

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
  virtual bool threadCanExit(tid_t tid) const = 0;

  /**
   * @brief Removes the given thread from the management
   * of the condition variable
   *
   * After calling `wakeThread()` on the policy, the supplied thread
   * is no longer managed by the policy and all references to the
   * thread are removed from the condition variable. Conceptually, the
   * thread has now "awoken" and has "escaped" the condition variable
   *
   * @note after the given thread has awoken, the criterion for
   * whether a thread is allowed to exit may have changed. Thus the
   * return typ
   *
   * @param tid the thread to wake up and allow to escape from the
   * condition variable
   *
   * @throws invalid_thread_wakeup_exception if the thread is now
   * allowed to exit the condition variable
   */
  virtual void wakeThread(tid_t tid) = 0;

public:

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
      return "Attempted to wake an inelligible thread on a condition "
             "variable";
    }
  };
};

} // namespace mcmini

using MCConditionVariablePolicy =
  mcmini::ConditionVariableWakeupPolicy;

#endif // INCLUDE_MCMINI_MISC_MCCONDITIONVARIABLEWAKEUPPOLICY_HPP
