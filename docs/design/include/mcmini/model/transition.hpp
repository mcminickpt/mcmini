#pragma once

#include <cstdint>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/optional.hpp"
#include "mcmini/model/state.hpp"

namespace mcmini::model {
/**
 * @brief A function over states of a program.
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
 * needs to exist to describe a transition. Such a transition is said to
 * _exist_ at that state, even if it is not technically enabled. Only a subset
 * of all transitions defined at a state `s` are actually enabled there (with at
 * most one transition per thread).
 */
class transition {
 public:
  // TODO: Defining categories could be interesting here. Such a definition will
  // be needed for example to register new transitions at launch-time. Assigning
  // it to the class members will be a bit more difficult, but it should be
  // possible nonetheless. This would speed up dependency checking.
  using category = uint32_t;
  category get_category();

  /**
   * @brief A result of a modification to a state.
   *
   * There are two possible outcomes attempting to apply a transition function:
   *
   * 1. Either the transition _exists_ at this state and is thus defined
   * 2. Or else the transition
   */
  enum class status { exists, disabled };

  /**
   * @brief Attempts to produce a state _s'_ from state _s_ through the
   * application of this transition function on argument _s_
   *
   * Recall that a transition is merely a function over the states of a
   * concurrent system. Recall further that a transition is only a _partial_
   * function: it need not be defined in all states of the concurrent system.
   * This method thus returns
   *
   * @param s the state to pass as an argument to the transition.
   * @returns the resulting state _s'_ that would be produced if this transition
   * were applied to state _s_ if such a transition is defined at _s_, and
   * otherwise the empty optional.
   */
  mcmini::optional<std::unique_ptr<state>> apply_to(const state &s) {
    std::unique_ptr<mutable_state> s_prime = s.mutable_clone();
    return modify(*s_prime) == status::exists
               ? mcmini::optional<std::unique_ptr<state>>(std::move(s_prime))
               : mcmini::optional<std::unique_ptr<state>>();
  }

  virtual status modify(mutable_state &state) = 0;
  virtual std::string to_string() const = 0;
};

}  // namespace mcmini::model