#pragma once

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

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
  local_linux_process() = default;
  ~local_linux_process() {
    kill(pid, SIGUSR1); /* TODO: React to errors here */
    waitpid(pid, NULL, 0);
  }
  local_linux_process(pid_t pid) : pid(pid) {}
};
}  // namespace mcmini::real_world