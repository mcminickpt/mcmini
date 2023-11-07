#pragma once

#include "mcmini/model/program.hpp"
#include "mcmini/real_world/process.hpp"

namespace mcmini::model_checking {

/**
 * An abstraction representing a function which can formally verify
 * the correctness of a program whose visible operations are modeled under the
 * mcmini model.
 *
 * A _model checking algorithm_ is nothing more than a function. Give
 *
 *    *  A McMini model-checking algorithm
 * operates
 */
class algorithm {
 public:
  struct callbacks {
   public:
    virtual void encountered_deadlock_in(const mcmini::model::program &);
    virtual void encountered_crash_in(const mcmini::model::program &);
    virtual void encountered_data_race_in(const mcmini::model::program &);
  };

  /**
   * @brief Run this model-checking algorithm starting from _initial_state_
   * which models _initial_process_.
   *
   * To verify the correctness of a program using explicit-state model checking,
   * any verification algorithm must effectively investigate all possible states
   * of that program. Each particular state of the program is represented in
   * McMini by an instance `mcmini::model::program`.
   *
   * The `mcmini::model::program` conceptually represents the "history" of
   * changes that some process underwent. The process whose states are described
   * by any program is represented `mcmini::real_world::process`. The proce
   *
   *
   *
   * @param initial_state the state from which the algorithm should begin its
   * search. This is often referred to as `s_0` or the "initial state" in the
   * model-checking literature.
   * @param corresponding_process a process whose execution is suspeded and
   * whose memory is modeled by `initial_state`. The algorithm will repeatedly
   * create new processes from this one as part of its exploration.
   * @invariant: A _critical_ invariant is that _corresponding_process_ be
   * modeled by _initial_state_. McMini model-checking algorithms rely _solely_
   * on the model to make determinations about how to explore the state space of
   * process _corresponding_process_.
   * @param callbacks a set of functions which are invoked as verification takes
   * place. The callbacks will be invoked when the algorithm encounters the
   * following violations:
   *
   * - when a state _s_ is encountered that is in _deadlock_.(i.e. there is no
   * way to make any more progress). Formally, this means there are no enabled
   * transitions at state _s_.
   * - when a state _s_ has a data race.
   * - if, at any point during verification, one of the corresponding processes
   * crashes.
   */
  virtual void verify_from(
      const mcmini::model::program &initial_state,
      const mcmini::real_world::process &corresponding_process,
      const callbacks &callbacks) = 0;
};

};  // namespace mcmini::model_checking