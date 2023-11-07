#pragma once

#include <stdint.h>

#include <memory>

namespace mcmini::model {

/**

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