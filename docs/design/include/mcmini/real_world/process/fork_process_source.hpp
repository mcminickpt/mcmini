#pragma once

#include <string>

#include "mcmini/defines.h"
#include "mcmini/misc/volatile_mem_streambuf.hpp"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/shm.hpp"

namespace real_world {

/**
 * @brief A factory which produces `real_world::local_linux_process` by
 * `fork()`-ing this process followed by an `exec()` into the targeted process
 * located at `target` with `libmcmini.so` preloaded.
 *
 * A `fork_process_source` is responsible for creating new processes by forking
 * this process and repeatedly `exec()`-ing into a new one.
 */
class fork_process_source : public process_source {
 private:
  // The name of the program which we should exec() into with libmcmini.so
  // preloaded.
  std::string target_program;  // NOTE: Favor std::filesystem::path if C++17
                               // is eventually supported
                               // Alternatively, have McMini conditionally
                               // compile a std::filesystem::path e.g.
  static std::unique_ptr<shared_memory_region> rw_region;
  static volatile_mem_streambuf
      runner_mailbox_bufs[MAX_TOTAL_THREADS_IN_PROGRAM];
  static runner_mailbox_stream
      *runner_mailbox_streams[MAX_TOTAL_THREADS_IN_PROGRAM];
  static void initialize_shared_memory();

  void setup_ld_preload();

 public:
  fork_process_source(std::string target_program);
  std::unique_ptr<process> make_new_process() override;
};

}  // namespace real_world