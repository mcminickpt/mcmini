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

  // NOTE: At the moment, each process has the entire view of the
  // shared memory region at its disposal. If desired, an extra layer could be
  // added on top which manages allocating slices of a `shared_memory_region`
  // and "allocates" them to different processes. This would look similar to
  // `malloc()/free()`, where the `free()` would be triggered by the destructor
  // of the slice. This is overly complicated at the moment, and we simply
  // restrict the number of proxy processes to one.

 public:
  local_linux_process() : local_linux_process(-1) {}
  local_linux_process(pid_t pid);
  virtual ~local_linux_process();
  volatile runner_mailbox *execute_runner(runner_id_t) override;
};
}  // namespace real_world
