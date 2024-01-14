#pragma once

#include <unordered_set>

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
  state_sequence state_seq;
  transition_sequence trace;
  pending_transitions next_steps;

 public:
  using runner_id_t = uint32_t;

  program(state &&initial_state, pending_transitions &&initial_first_steps);
  program(program &&) = default;
  program(const program &) = delete;

  const state_sequence &get_state_sequence() const { return this->state_seq; }
  const transition_sequence &get_trace() const { return this->trace; }

  /**
   * @brief Returns a list of runners which are currently enabled.
   */
  std::unordered_set<runner_id_t> get_enabled_runners() const {
    std::unordered_set<runner_id_t> enabled_runners;
    for (const auto &runner_and_t : this->next_steps) {
      if (runner_and_t.second->is_enabled_in(state_seq)) {
        enabled_runners.insert(runner_and_t.first);
      }
    }
    return enabled_runners;
  }

  void execute_runner(runner_id_t p,
                      std::unique_ptr<transition> new_transition) {
    const transition *next_s_p = next_steps.get_transition_for_runner(p);

    if (next_s_p) {
      this->state_seq.follow(*next_s_p);
      this->next_steps.displace_transition_for(p, std::move(new_transition));
    } else {
      // TODO: Handle the case where `p` doesn't exist. Perhaps this function
      // should return a `mcmini::result<>` type.
      throw std::runtime_error(
          "Attempted to execute a runner whose transition was not currently "
          "enabled");
    }
  }
};
//

};  // namespace mcmini::model