#pragma once

#include "mcmini/model/program.hpp"

namespace mcmini::verification::modelchecking {

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
   * Reference
   */

  /**
   * @brief Run this verification algorithm starting from _initial_state_
   * which models _initial_process_.
   *
   * To verify the correctness of a program using explicit-state model checking,
   * any verification algorithm must effectively investigate all possible states
   * of that program. Each particular state of the program is modeled with
   * Since how a program will ultimately evolve is unknown past
   * the next immediate action that can be taken by any thread i
   * investigating
   *
   *  +-------+                             +----------------------+
   * | model |     kept synchronized       |     corresponding    |
   * |  of   |  <------------------------> |      process         |
   * |  the  |                             +----------------------+
   * | world |                            |        |        |
   * +-------+                           +---+    +---+    +---+
   *                                    |t_0j|   |t_1j|   |t_2j| ...
   *                                    +---+    +----+   +----+
   *                                      /        /        /
   *                                    +---+    +---+    +---+
   *                                   |????|   |????|   |????| ...
   *                                   +---+    +----+   +----+
   *
   *
   * The initial state consists of a
   *
   *
   * @param initial_state a model of
   * @param corresponding_process
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
      const mcmini::verification::process &corresponding_process,
      const callbacks &callbacks) = 0;
};

struct depth_first_algorithm : public algorithm {
 public:
};

struct dpor_algorithm : public depth_first_algorithm {};

struct brute_force_algorithm : public depth_first_algorithm {};

};  // namespace mcmini::verification::modelchecking