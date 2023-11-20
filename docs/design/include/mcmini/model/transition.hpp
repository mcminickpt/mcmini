#pragma once

#include <cstdint>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/optional.hpp"
#include "mcmini/model/state.hpp"

namespace mcmini::model {
/**
 * @brief A description of how to transform a state.
 *
 * We give the formal definition of a transition from Rodriguez et al. 2018
 * UDPOR
 *
 * """
 * A system is a tuple M <`Σ, T, S_0> formed by a set Σ of global states, a set
 * T of transitions and some initial global state ˜ s > Σ. Each transition t:
 * Σ --> Σ in T is a partial function accounting for how the occurrence of t
 * transforms the state of M. A transition t > T is enabled at a state s if t(s)
 * is defined. Such t can fire at s, producing a new state s'. We let
 * enabled(s) denote the set of transitions enabled at s.
 * """
 *
 * The formal definition clarifies that a transition is _enabled_ at some state
 * `s` iff t(s) is defined at the state. The implementation takes a slightly
 * different approach:
 *
 * - a transition in McMini _may be defined_ in a state `s` but may be
 * _disabled_ in that state; that is, McMini distinguishes between a transition
 * which _cannot exist_ in a given state (e.g. a pthread_mutex_lock() on some
 * mutex which isn't present) and that which _should not exist_ formally but
 * needs to exist to describe a transition. Such a transition is said to be
 *
 * Transitions ensure that we can
 */
class transition {
 public:
  using category = uint32_t;
  category get_category();

  enum class status { enabled, disabled };

  /**
   * @brief Apply the given transition to the state _s_.
   *
   *
   */
  mcmini::optional<std::unique_ptr<state>> apply_to(const state &s) {
    auto s_copy = s.clone();
    return modify(*s_copy) == status::enabled
               ? mcmini::optional<std::unique_ptr<state>>(std::move(s_copy))
               : mcmini::optional<std::unique_ptr<state>>();
  }

  /**
   *
   */
  virtual status modify(state &s) = 0;
};

}  // namespace mcmini::model