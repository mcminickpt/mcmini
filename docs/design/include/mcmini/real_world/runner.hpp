#pragma once

#include "mcmini/model/transition.hpp"

/**
 * @brief Manages the runtime execution of entities that exist outside of the
 * McMini model checker.
 *
 *  A `runner` is a  bridge between the _model_ in McMini's architecture
 * execution units it interacts with. It is a proxy for a single unit that
 * McMini takes control over during verification. It can represent a single
 * thread, multiple threads, a process, multiple processes, or entire machines.
 * It is responsible for enacting the transitions whose execution McMini has
 * _simulated_ during verification and informing the model checker of where
 * execution has subsequently been suspended (when the entity has encountered
 * its next transition).
 */
class runner {
 private:
 public:
  using runner_id_t = uint32_t;

  /**
   *
   *
   */
  virtual std::istream &continue_to_next_transition(
      const model::transition &current_transition) = 0;
  virtual ~runner() = default;
};