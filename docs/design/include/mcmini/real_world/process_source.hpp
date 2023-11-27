#pragma once

#include "mcmini/real_world/process.hpp"

namespace mcmini::real_world {

/**
 * @brief An object which is responsible for repeatedly spawning new processes.
 *
 * A `mcmini::real_world::process_source` manages the creation of new processes
 * from a fixed point
 */
class process_source {
 public:
  /**
   * @brief TODO: Document this
   */
  virtual std::unique_ptr<process> make_new_process() = 0;
};

}  // namespace mcmini::real_world