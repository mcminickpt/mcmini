#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "mcmini/model/defines.hpp"
#include "mcmini/model/transitions/transition_sequence.hpp"

namespace model_checking {
/**
 * @brief A multi-dimensional vector used to keep track of "time" between
 * dependencies in a model checking trace
 *
 * A clock vector is simply a vector of integer components. They are used in
 * message-passing systems to determine if an event in a distributed system
 * "causally-happens-before" another.  But clock vectors are more generally
 * applicable: they can be used to determine a "happens-before" partial order
 * even in the absence of explicit "send" and "receive" messages. McMini, for
 * example, uses clock vectors to determine the "happens-before" relation among
 * transitions in a transition sequence
 *
 * A `clock_vector` implicitly maps a thread id it does not contain to a default
 * value of `0`. Thus, a `clock_vector` is "lazy" in the sense that new threads
 * are "automatically" mapped without needing to be explicitly added the clock
 * vector when the thread is created
 */
struct clock_vector final {
 private:
  std::unordered_map<runner_id_t, model::transition_sequence::index> contents;

 public:
  using index = model::transition_sequence::index;
  static constexpr index invalid = -1;

  clock_vector() = default;
  clock_vector(const clock_vector &) = default;
  clock_vector(clock_vector &&) = default;
  clock_vector &operator=(const clock_vector &) = default;
  clock_vector &operator=(clock_vector &&) = default;

  /**
   * @brief The number of components in this clock vector
   *
   * The `clock_vector` is "lazy" in the sense that
   * new components must be explicitly added to the
   * clock vector: the clock vector is not of an
   * initial, fixed size. Thus two `clock_vector`s may
   * have different sizes
   *
   * @return the number of elements in the clock vector
   */
  uint32_t size() const { return this->contents.size(); }

  index &operator[](runner_id_t tid) {
    // NOTE: The `operator[]` overload of
    // unordered_map will create a new key-value
    // pair if `tid` does not exist and will use
    // a _default_ value for the value (0 in this case)
    // which is actually what we want here
    return this->contents[tid];
  }

  /**
   * @brief Retrieves the value for the thread if mapped by this clock vector
   *
   * @param tid the id of the thread to check for a mapping
   * @return the value for the thread if mapped by this clock
   * vector or `0` if the clock vector does not have an explicit mapping for the
   * thread
   */
  index value_for(runner_id_t tid) const {
    const auto iter = this->contents.find(tid);
    if (iter != this->contents.end()) return iter->second;
    return 0;
  }

  bool has_explicit_mapping_for(runner_id_t rid) const {
    return this->contents.count(rid) != 0;
  }

  bool has_implicit_mapping_for(runner_id_t rid) const {
    return this->contents.count(rid) == 0;
  }

  /**
   * @brief Computes a clock vector whose components
   * are larger than the components of both of
   * the given clock vectors
   *
   * The maximum of two clock vectors is definied to be the clock vector whose
   * components are the maxmimum of the corresponding components of the
   * arguments. Since the `clock_vector` class is "lazy", the two clock vectors
   * given as arguments may not be of the same size. The resultant clock vector
   * has components as follows:
   *
   * 1. For each thread that each clock vector maps, the resulting clock vector
   * maps that thread to the maxmimum of the values mapped to the thread by each
   * clock vector
   *
   * 2. For each thread that only a single clock vector maps, the resulting
   * clock vector maps that thread to the value mapped by the lone clock vector
   *
   * The scheme is equivalent to assuming that an unmapped thread by any one
   * clock vector is implicitly mapped to zero
   *
   * @param cv1 the first clock vector
   * @param cv2  the second clock vector
   * @return clock_vector a clock vector whose components are at least as large
   * as the corresponding components of each clock vector and whose size is
   * large enough to contain the union of components of each clock vector
   */
  static clock_vector max(const clock_vector &cv1, const clock_vector &cv2);
};
}  // namespace model_checking
