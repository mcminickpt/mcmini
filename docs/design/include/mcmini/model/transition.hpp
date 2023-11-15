#pragma once

namespace mcmini::model {

/**
 * @brief A description of how to transform a state.
 *
 */
class read_only_transition {
 public:
};

/**
 * @brief A description of how to transform a state.
 *
 * We give the formal definition of a transition (Rodriguez et al. 2018 UDPOR)
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
 * mutex which isn't present).
 *
 * This ensures that we can track what each thread.
 */
class transition : public read_only_transition {
 public:
};

}  // namespace mcmini::model