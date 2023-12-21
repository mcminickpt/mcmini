#pragma once

#include "mcmini/real_world/process.hpp"

namespace mcmini::real_world {

/**
 * @brief An object which is responsible for repeatedly spawning new processes.
 *
 * A `mcmini::real_world::process_source` manages the creation of new processes
 * from a known fixed point in the progression of the processes. It is important
 * that a process source document at which point in the execution the processes
 * it creates will be for each new source.
 */
class process_source {
 public:
  virtual std::unique_ptr<process> make_new_process() = 0;
};

}  // namespace mcmini::real_world