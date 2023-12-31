#pragma once

#include <string>

#include "mcmini/real_world/process_source.hpp"

namespace mcmini::real_world {

/**
 * @brief A factory which produces `mcmini::real_world::local_linux_process` by
 * `fork()`-ing this process followed by an `exec()` into the targeted process
 * located at `target`.
 *
 * A `fork_process_source` is responsible for creating new processes.
 *
 */
class fork_process_source : public process_source {
 private:
  pid_t target_to_send_a_sigusr1_to_repeatedly;

 public:
  std::unique_ptr<process> make_new_process() override;
};

}  // namespace mcmini::real_world