#pragma once

#include <stdint.h>

#include <memory>

namespace mcmini::model {

/**
 * @brief A placeholder which represents a snapshot of something that multiple
 * threads interact with to communicate with each other in a program.
 *
 * A _visible object_, from the perspective of a model checker, are those
 * portions of a program which are semantically interesting with respect to
 * verification. Threads in a program interact with one another by communicating
 * somehow through something which is accessible to each thread communicating
 * (hence the idea of "visible").
 */
class visible_object_state {
 public:
  /**
   * Create a copy of this state.
   *
   * @return a pointer to a new object representing an object with the same
   * state.
   */
  virtual std::unique_ptr<visible_object_state> clone() = 0;
};

}  // namespace mcmini::model