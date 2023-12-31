#pragma once

#include "mcmini/model/pending_transitions.hpp"
#include "mcmini/model/state/state_sequence.hpp"
#include "mcmini/model/transitions/transition_sequence.hpp"

namespace mcmini::model {

/**
 * @brief A capture of the states visited in a particular branch of the state
 * space of a program undergoing verification; a trace describing how the
 * current state has come about as a sequence of transitions; and the
 * immediately visible "next steps" that can be taken by the program in its
 * current state.
 *
 * A program is an interface between a live process running on the CPU under
 * control of the operating system and the view of that process from the
 * perspective of the verifier. A _mcmini::model::program_ is a container that
 * comprises of the following three components:
 *
 * - a sequence `S` of transitions that have occurred called a _trace_.
 * - a model of the state of the program undergoing verification after each
 * point `t` in trace `S`.
 * - a mapping between the individual execution units of the program ("threads")
 * and the next thread routine ("transition") that thread will run once
 * executed.
 *
 * Each component corresponds directly to the theoretical models described in
 * Flanagan et al., Rodriguez et al., etc; specifically
 *
 * - the sequence `S` of transitions is some linearization of transitions that
 * the given program has followed to reach its current state.
 * - the model of the program at each point models how the state of the program
 * at each point during the sequence (s_0, s_1, ..., s_N). Each transition `t_i
 * in `S` describes how the program went from state `s_i` to `s_(i+1)`.
 * - the mapping of execution units to threads represents `next(s_N, p)`.
 *
 * Conceptually, the program's _current state_ is that state reached from
 * executing the sequence `S` of transitions in the order they appear from the
 * initial state `s_0` of the program. This is sometimes represented as
 * `state(S)` where `S` is some transition sequence and `s_0` is assumed to be
 * implied.
 */
class program {
 private:
  state_sequence states;
  transition_sequence trace;
  pending_transitions next_steps;

 public:
  using runner_id_t = uint32_t;

  program(state &&initial_state, pending_transitions &&initial_first_steps);
};
//

};  // namespace mcmini::model