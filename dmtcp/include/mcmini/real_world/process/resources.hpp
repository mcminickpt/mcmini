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
  static xpc_resources &get_instance() {
    static xpc_resources resources;
    return resources;
  }
  shared_memory_region *get_rw_region() const { return this->rw_region.get(); }
  void reset_binary_semaphores_for_new_process();
};

}  // namespace real_world
