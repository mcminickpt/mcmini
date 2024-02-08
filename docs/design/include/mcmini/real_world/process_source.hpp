#pragma once

#include "mcmini/real_world/process.hpp"

namespace real_world {

/**
 * @brief An object which is responsible for repeatedly spawning new processes.
 *
 * A `real_world::process_source` manages the creation of new processes
 * from a known fixed point in the progression of the processes.
 *
 * It is important that subclasses document at which point during the execution
 * the processes it creates are located. McMini relies on reproducibly
 * re-creating previous program states.
 */
class process_source {
 public:
  /**
   * @brief Spawn a new process starting from the the fixed point of this
   * process source.
   *
   * @returns a handle to a new process ready to execute from the startin point
   * of this process source, or `nullptr` if the process could not be created.
   *
   * NOTE: Returning a `result` type is better here -> the source would be able
   * to tell us what the issue was and would make for better error outputs.
   */
  virtual std::unique_ptr<process> make_new_process() = 0;
  virtual ~process_source() = default;
};

}  // namespace real_world