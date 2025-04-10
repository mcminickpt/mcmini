/* File copied from classic mcmini */
#pragma once

#include "mcmini/defines.h"

#include <initializer_list>
#include <set>
#include <vector>



/**
 * @brief  A set of threads eligible to escape a condition
 * variable
 * 
 * A *wake group* is a set of threads for which the transition
 * `pthread_cond_wait()` is enabled for those threads by virtue of
 * having been sleeping on the condition variable at the time a
 * signal/broadcast was sent to the condition variable.
 * 
 * A wake group represents a mutually-exclusive group of threads which
 * can consume a signal. The semantics of a condition variable are
 * such that signal consumption and the waking of a thread are atomic;
 * that is, the act of consuming the signal and waking the threads is
 * atomic.
 * 
 * Only a single thread can wake from a wake group created by a
 * signal message, while all threads in a broadcast wake group are
 * allowed to wakeup
 */

struct WakeGroup final : public std::vector<runner_id_t> {
public:

  WakeGroup(WakeGroup &&)                 = default;
  WakeGroup(const WakeGroup &)            = default;
  WakeGroup &operator=(const WakeGroup &) = default;
  WakeGroup &operator=(WakeGroup &&)      = default;

  explicit WakeGroup(const std::vector<runner_id_t> &vec)
    : std::vector<runner_id_t>(vec)
  {}

  explicit WakeGroup(std::initializer_list<runner_id_t> list)
    : std::vector<runner_id_t>(std::move(list))
  {}

  template<class InputIt>
  WakeGroup(InputIt first, InputIt last)
    : std::vector<runner_id_t>(first, last)
  {}

  /**
   * @brief Whether or not the given thread is contained in the group
   * 
   * @param tid the thread to test for membership in the wake group
   * @return true if the thread is contained in the wake group
   * @return false if the thread is not contained in the wake group
   */
  bool contains(runner_id_t tid) const;

  /**
   * @brief Removes the given thread _tid_ from the group
   * 
   * @param tid the thread to remove from the group
   */
   void remove_candidate_thread(runner_id_t tid);

  /**
   * @brief
   *
   * @param tid
   */
  void wake_thread(runner_id_t tid);
};
