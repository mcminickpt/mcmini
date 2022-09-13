#ifndef INCLUDE_MCMINI_MISC_COND_MCWAKEGROUP_HPP
#define INCLUDE_MCMINI_MISC_COND_MCWAKEGROUP_HPP

#include "mcmini/MCShared.h"
#include "mcmini/misc/MCOptional.h"
#include <cstdlib>
#include <set>
#include <vector>

namespace mcmini {

/**
 * @brief  A set of threads elligible to escape a condition
 * variable
 *
 * A *wake group* is a set of threads for which the transition
 * `pthread_cond_wait()` is enabled for those threads by virtue of
 * having been sleeping on the condition variable at the time a
 * signal/broadcast was sent to the condition variable. There are two
 * kinds of wake groups, corresponding to the two messages which can
 * be sent to a condition variable:
 *
 * 1. A signal wake group represents threads sleeping on the condition
 * variable before a signal message was sent to the condition variable
 *
 * 2. A broadcast wake group represents threads sleeping on the
 * condition variable before a broadcast message was sent to the
 * condition variable
 *
 * Only a single thread can wake from a wake group created by a
 * signal message, while all threads in a broadcast wake group are
 * allowed to wakeup
 */
struct WakeGroup final {
  /**
   * @brief The
   */
  enum class Type { signal, broadcast } type = Type::signal;

  WakeGroup() = default;
  WakeGroup(const std::vector<tid_t> &vec);
  WakeGroup(Type type);
  WakeGroup(Type type, const std::vector<tid_t> &);

  /**
   * @brief Whether or not the given thread is contained in the group
   *
   * @param tid the thread to test for membership in the wakegroup
   * @return true if the thread is contained in the wakegroup
   * @return false if the thread is not contained in the wakegroup
   */
  bool containsThread(tid_t tid) const;

  /**
   * @brief
   *
   * @param tid
   */
  void removeCandidateThread(tid_t tid);

  /**
   * @brief
   *
   * @param tid
   */
  void wakeThread(tid_t tid);

  MCOptional<tid_t>
  threadAtIndex(int index) const
  {
    if (index < 0 || index >= threads.size())
      return MCOptional<tid_t>::nil();
    return MCOptional<tid_t>::some(threads[index]);
  }

  MCOptional<tid_t>
  top() const
  {
    return this->threadAtIndex(0);
  }

  MCOptional<tid_t>
  back() const
  {
    return this->threadAtIndex(this->threads.size() - 1);
  }

  inline ssize_t
  size() const
  {
    return threads.size();
  }

  inline bool
  empty() const
  {
    return threads.empty();
  }

private:

  std::vector<tid_t> threads;
  std::set<tid_t> threadSet;
};
} // namespace mcmini

#endif // INCLUDE_MCMINI_MISC_COND_MCWAKEGROUP_HPP
