#pragma once

#include <cstdint>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/optional.hpp"
#include "mcmini/model/state.hpp"

namespace model {

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
 * _exist_ at that state, even if it is not technically enabled there. Only a
 * subset of all transitions that _exist_ at a state `s` are actually enabled
 * there (with at most one transition per thread).
 *
 * Formally, if a transition is defined in a state `s`, it means it's enabled
 * there according to the formal definition. However, the definition of
 * `model::transition` allows the transition to produce a state _even if
 * the process which is set to execute the transition isn't truly in a position
 * where the transition is being executed_. Transitions perform _look ups_ on
 * the particular state they are given
 *
 * For example, consider a transition "post(sem)" which takes a visible object
 * (a semaphore) with id `sem` as an argument. The sempahore `sem` may exist in
 * several states, say `s_1, s_2, and s_3`. Suppose thread 1 executes
 * "post(m)" and brings the concurrent system from state `s_1` to `s_2`, and
 * suppose that in state `s_2` thread 1 executes "wait(sem)". In state `s_2`,
 * the transition "thread 1 executes post(sem)" is _not_ enabled since thread 1
 * is _not_ executing a `post(sem)`. However, the implementation of "thread 1
 * executes post(sem)"  _would be defined_ in state `s_2`. In other words, a
 * `model::state` _excludes_ the state of the _processes_; that is the
 * responsibility of `model::program`.
 */
class transition {
 public:
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
   * otherwise `nullptr`
   */
  std::unique_ptr<state> apply_to(const state& s) const {
    std::unique_ptr<mutable_state> s_prime = s.mutable_clone();
    return modify(*s_prime) == status::exists
               ? std::unique_ptr<state>(std::move(s_prime))
               : std::unique_ptr<state>();
  }

  /**
   * @brief Whether the transition is defined in the given state.
   *
   * @param s the state to determine if this transition is enabled.
   */
  bool is_enabled_in(const state& s) const { return apply_to(s) != nullptr; }

  /**
   * @brief A result of a modification to a state.
   *
   * There are two possible outcomes attempting to apply a transition
   * function:
   *
   * 1. Either the transition _exists_ at this state and is thus defined at
   * the given state.
   * 2. Or else the transition is _not_ defined as this state and the
   * transition is disabled.
   */
  enum class status { exists, disabled };

  /**
   * @brief Fire the transition as if it were run from state _state_.
   *
   * A transition is said to _fire_ at a state `s` of a concurrent system if
   * that transition is enabled there and the system moves into the state `s'`
   * the transition maps to `s`
   *
   * @param s the state taken as an argument to the transition function; the
   * state which this transition is "applied to"
   * @returns whether the transition were enabled in this state. If the
   * transition is _not_ enabled in this state, the contents of state is
   * undefined; otherwise, the object `state` will represent the new state `s'`
   * reached by the system after this transition fires.
   */
  virtual status modify(mutable_state& s) const = 0;

  // TODO: Add a serialization method here later if we want to support
  // transitions sending different return values to the wrapper functions that
  // they represent. The signature would be something like
  // `virtual void serialize(std::ostream&, const
  // coordinator::context::model_to_system_map&) const = 0;`

  virtual std::string to_string() const = 0;
  virtual ~transition() = default;
};

}  // namespace model