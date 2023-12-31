#pragma once

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/model/program.hpp"

namespace mcmini::model_checking {

/**
 * @brief An abstraction representing a function which can formally verify
 * the correctness of a program modeled under McMini.
 *
 * A _model checking algorithm_ is nothing more than a function. Give
 *
 */
class algorithm {
 public:
  // TODO: Eventually we may want to pass more information to the callbacks
  // (e.g. the algorithm itself or the coordinator) to provide detailed printing
  // information.
  struct callbacks {
   public:
    virtual void encountered_deadlock_in(const mcmini::model::program &) {}
    virtual void encountered_crash_in(const mcmini::model::program &) {}
    virtual void encountered_data_race_in(const mcmini::model::program &) {}
  };

  /**
   * @brief Run this model-checking algorithm using coordinator _coordinator_.
   *
   * To verify the correctness of a program using explicit-state model checking,
   * any verification algorithm must investigate all possible states of that
   * program. Each particular state of the program is represented in McMini by
   * an instance `mcmini::model::state`. A state plus the collection of
   * transitions which are defined at that state is captured in a
   * `mcmini::model::program`.
   *
   * The `mcmini::model::program` conceptually represents the "history" of
   * changes that some process underwent. The process whose states are described
   * by the program is represented `mcmini::real_world::process`.
   *
   * @param coordinator the coordinator which manages the syncrhonization of the
   * program and the processes which those programs represent.
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
  virtual void verify_using(mcmini::coordinator &coordinator,
                            const callbacks &callbacks) = 0;

  void verify_using(mcmini::coordinator &coordinator) {
    callbacks no_callbacks;
    this->verify_using(coordinator, no_callbacks);
  }
};

};  // namespace mcmini::model_checking