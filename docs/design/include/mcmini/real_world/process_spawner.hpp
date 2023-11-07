#pragma once

#include "mcmini/real_world/process.hpp"

namespace mcmini::real_world {

class process_spawner {
 public:
  process make_new_process();
};

}  // namespace mcmini::real_world