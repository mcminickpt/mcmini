#pragma once

#include <memory>

#include "mcmini/real_world/shm.hpp"

namespace real_world {

/**
 * @brief A singleton holding shared resources between the McMini process and
 * the other processes it interacts with.
 */
struct xpc_resources {
 private:
  xpc_resources();
  std::unique_ptr<shared_memory_region> rw_region = nullptr;

 public:
  /// @brief The special port used by the `dmtcp_coordinator` which exists
  /// specifically for aiding McMini with creating new target branches
  /// after restart.
  static const int dmtcp_coordinator_port = 7778;
  static xpc_resources &get_instance() {
    static std::unique_ptr<xpc_resources> resources = nullptr;
    if (!resources) {
      resources = std::unique_ptr<xpc_resources>(new xpc_resources());
    }
    return *resources;
  }
  shared_memory_region *get_rw_region() const { return this->rw_region.get(); }
  void reset_binary_semaphores_for_new_branch();
};

}  // namespace real_world
