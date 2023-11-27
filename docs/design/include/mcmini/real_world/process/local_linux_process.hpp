#pragma once

#include "mcmini/real_world/process.hpp"

namespace mcmini::real_world {

/**
 * @brief A proxy for a process running under Linux.
 *
 * A `mcmini::real_world::linux_process` is a local proxy for a process running
 * on the same machine.
 *
 */
class local_linux_process : public process {
 private:
  pid_t pid;

 public:
  void execute_thread(tid_t tid) override;
};
}  // namespace mcmini::real_world