#pragma once

#include <functional>
#include <unordered_set>

#include "mcmini/model/defines.hpp"
#include "mcmini/model/pending_transitions.hpp"
#include "mcmini/model/state/state_sequence.hpp"
#include "mcmini/model/transitions/transition_sequence.hpp"

namespace model {

/**
 * @brief A capture of the states visited in a particular branch of the state
 * space of a program undergoing verification; a trace describing how the
 * current state has come about as a sequence of transitions; and the
 * immediately visible "next steps" that can be taken by the program in its
 * current state.
 *
 * A program is an interface between a live process running on the CPU under
 * control of the operating system and the view of that process from the
 * perspective of the verifier. A _model::program_ is a container that
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
  using runner_id_t = ::runner_id_t;
  program();
  program(const state &initial_state,
          pending_transitions &&initial_first_steps);
  program(program &&) = default;
  program(const program &) = delete;

  std::unordered_set<runner_id_t> get_enabled_runners() const;
  size_t get_num_runners() const { return next_steps.size(); }
  mutable_state &get_current_state() { return this->state_seq; }
  pending_transitions &get_pending_transitions() { return this->next_steps; }
  const state_sequence &get_state_sequence() const { return this->state_seq; }
  const transition_sequence &get_trace() const { return this->trace; }
  const pending_transitions &get_pending_transitions() const {
    return this->next_steps;
  }
  const transition *get_pending_transition_for(runner_id_t rid) const {
    return next_steps.get_transition_for_runner(rid);
  }

  using runner_generation_function =
      std::function<const model::transition *(model::state::runner_id_t)>;

  /// @brief Introduce a new object into the model with initial state `s`
  /// @param s the initial state of the new object to add to the model
  /// @return the id assigned to the object in the model
  state::objid_t discover_object(const visible_object_state *s);

  /// @brief Introduce a new runner into the model with initial state `s`
  /// @param s the initial state of the new object to add to the model
  /// @param f a function which, when passed the id that will be vended to the
  /// runner, produces the first transition that runner is executing in the
  /// model (i.e. the very first pending operation).
  /// @return the id assigned to the runner.
  state::runner_id_t discover_runner(const runner_state *s,
                                     runner_generation_function f);

  /// @brief Introduce a new runner into the model with initial state `s`
  /// running transition `next_transition`
  /// @param s the initial state of the new object to add to the model
  /// @param next_transition the next operation the runner will execute in
  /// state `s`
  /// @return the id assigned to the runner.
  state::runner_id_t discover_runner(const runner_state *s,
                                     const transition *next_transition);

  /// @brief Model the execution of runner `p` and replace its next operation
  /// with `next_pending_operation`.
  ///
  /// @param p the id of the runner whose next transition should be simulated.
  /// @param npo (short for `next_pending_operation`) the next transition this
  /// is pending after `p` executes; that is, this is the transition that `p`
  /// will run in the state modeled _after_ `next_s_p` is executed.
  /// @throws an runtime exception is raised if the transition replacing
  /// `next_s_p` is not executed by `p` or if `p` is not currently known to the
  /// model.
  void model_execution_of(runner_id_t p, const transition *npo);

  /// @brief Restore the model as if it were `n` steps into execution.
  ///
  /// @param n the number of transitions to consider executed to result in the
  /// program model after the method has executed. Formally, if `t_0, t_1, ...,
  /// t_(n-1), ..., t_k` is contained in the current trace, then the state after
  /// this method is called will be `s_(n+1)` or `s_0` if `n = 0`.
  void restore_model_at_depth(uint32_t n);

  // MARK: Program State
  bool is_in_deadlock() const;
};
//

};  // namespace model
