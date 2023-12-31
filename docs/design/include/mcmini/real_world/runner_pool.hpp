#pragma once

#include "mcmini/real_world/runner.hpp"

namespace mcmini {

/**
 * @brief Manages the creation of new runners at runtime.
 *
 * A `runner_pool` is a centralized location for runners.
 *
 */
class runner_pool {
 public:
  virtual runner &new_runner();
};

}  // namespace mcmini