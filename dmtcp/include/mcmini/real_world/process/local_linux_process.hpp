#pragma once

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mcmini/real_world/process.hpp"
#include "mcmini/real_world/shm.hpp"

namespace real_world {

/**
 * @brief A proxy for a process running under Linux.
 *
 * A `real_world::linux_process` is a local proxy for a process running
 * on the same machine. Each instance manages a portion of shared memory that it
 * uses to communicate with the proxied process. The proxied process is assumed
 * to be under the control of `libmcmini.so`; otherwise, executing runners
 * results in undefined behavior.
 */
class local_linux_process : public process {
 private:
  pid_t pid;
  bool should_wait; /* Whether `wait(3)` should be called on destruction */

 public:
  pid_t get_pid() const override { return pid; }
  local_linux_process() : local_linux_process(-1) {}
  local_linux_process(pid_t pid) : local_linux_process(pid, true) {}
  local_linux_process(pid_t pid, bool should_wait);
  local_linux_process(const local_linux_process&) = delete;
  local_linux_process(local_linux_process&&);
  local_linux_process& operator=(const local_linux_process&) = delete;
  local_linux_process& operator=(local_linux_process&&);
  virtual ~local_linux_process();
  volatile runner_mailbox* execute_runner(runner_id_t) override;
};
}  // namespace real_world
