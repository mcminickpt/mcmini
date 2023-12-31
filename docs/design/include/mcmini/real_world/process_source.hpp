#pragma once

#include "mcmini/real_world/process.hpp"

namespace mcmini::real_world {

/**
 * @brief An object which is responsible for repeatedly spawning new processes.
 *
 * A `mcmini::real_world::process_source` manages the creation of new processes
 * from a known fixed point in the progression of the processes.
 *
 * It is important that subclasses document at which point during the execution
 * the processes it creates are located. McMini relies on reproducibly
 * re-creating previous program states.
 */
class process_source {
 public:
  virtual std::unique_ptr<process> make_new_process() = 0;
};

}  // namespace mcmini::real_world