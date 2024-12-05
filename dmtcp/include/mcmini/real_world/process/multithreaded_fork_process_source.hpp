#pragma once

#include <string>

#include "mcmini/defines.h"
#include "mcmini/forwards.hpp"
#include "mcmini/real_world/process/dmtcp_coordinator.hpp"
#include "mcmini/real_world/process/fork_process_source.hpp"

namespace real_world {

/**
 * @brief A factory which produces `real_world::local_linux_process` by
 * `multithreaded-fork()`-ing a templace process process named `target` with
 * `libmcmini.so` preloaded into it after restarting
 *
 * A `fork_process_source` is responsible for creating new processes by forking
 * and `exec()`-ing into a template process and then repeatedly forking
 * the new process to create new processes. The processes that are vended are
 * duplicates of the template process.
 */
class multithreaded_fork_process_source : public fork_process_source {
 private:
  dmtcp_coordinator coordinator_target;

 public:
  multithreaded_fork_process_source(const std::string &ckpt_file);
};

}  // namespace real_world
