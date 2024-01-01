#pragma once

#include <stdint.h>

#include <memory>

namespace mcmini::model {

/**
 * @brief A capture of the state of a particular visible object.
 *
 */
class visible_object_state {
 public:
  virtual ~visible_object_state();

  /**
   * Create a copy of this state.
   *
   * @return a pointer to a new object representing an object with the same
   * state.
   */
  virtual std::unique_ptr<visible_object_state> clone() const = 0;
};

}  // namespace mcmini::model