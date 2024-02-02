#pragma once

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mcmini/detail/volatile_mem_stream.hpp"
#include "mcmini/real_world/process.hpp"
#include "mcmini/real_world/shm.hpp"

namespace real_world {

/**
 * @brief A proxy for a process running under Linux.
 *
 * A `real_world::linux_process` is a local proxy for a process running
 * on the same machine.
 */
class local_linux_process : public process {
 private:
  pid_t pid;

  // TODO: We add the shared memory portion here for now and figure out the
  // "runner" bit later... Each local linux process will share the same static
  // memory region. Even in the runner model, the thread runners would each
  // share the memory region but it wouldn't be attached to the processes
  // themselves
  static shared_memory_region read_write_region;

 public:
  local_linux_process() = default;
  local_linux_process(pid_t pid) : pid(pid) {}
  virtual ~local_linux_process();

  std::istream &execute_runner(runner_id_t mcmini_runner_id) override;
};
}  // namespace real_world