#pragma once

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "mcmini/detail/volatile_mem_stream.hpp"
#include "mcmini/real_world/process.hpp"
#include "mcmini/real_world/shm.hpp"

namespace mcmini::real_world {

/**
 * @brief A proxy for a process running under Linux.
 *
 * A `mcmini::real_world::linux_process` is a local proxy for a process running
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
  static mcmini::detail::volatile_mem_stream &get_mem_stream_wrapper() {}


 public:
  local_linux_process() = default;
  virtual ~local_linux_process();
  local_linux_process(pid_t pid) : pid(pid) {}

  std::istream &execute_runner(runner_id_t mcmini_runner_id) override;

  // Initialize the shared memory region
  static void initializeSharedMemory(const std::string& shm_file_name, size_t region_size) {
    read_write_region = shared_memory_region(shm_file_name, region_size);

  }

};

}  // namespace mcmini::real_world