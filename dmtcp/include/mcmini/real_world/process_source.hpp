#pragma once

#include <stdexcept>

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
  struct process_creation_exception : public std::runtime_error {
    explicit process_creation_exception(const char *c)
        : std::runtime_error(c) {}
    explicit process_creation_exception(const std::string &s)
        : std::runtime_error(s) {}
  };

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

  /**
   * @brief Attempts to make a new process and raises a
   * `process_soprocess_creation_exception` on failure
   */
  std::unique_ptr<process> force_new_process() {
    auto p = make_new_process();
    if (!p) {
      throw process_creation_exception("Process creation failed");
    }
    return p;
  }

  virtual ~process_source() = default;
};

}  // namespace real_world
