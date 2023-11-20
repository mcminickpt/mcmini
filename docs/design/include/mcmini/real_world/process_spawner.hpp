#pragma once

#include "mcmini/real_world/process.hpp"

namespace mcmini::real_world {

/**
 * @brief An object which is responsible for repeatedly spawning new processes.
 *
 */
class process_spawner {
 public:
  process make_new_process();
};

}  // namespace mcmini::real_world