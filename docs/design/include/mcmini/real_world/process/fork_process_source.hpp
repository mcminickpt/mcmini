#pragma once

#include <string>

#include "mcmini/real_world/process_source.hpp"

namespace mcmini::real_world {

/**
 * @brief A factory which produces `mcmini::real_world::local_linux_process` by
 * `fork()`-ing this process followed by an `exec()` into the targeted process
 * located at `target` with `libmcmini.so` preloaded.
 *
 * A `fork_process_source` is responsible for creating new processes by forking
 * this process.
 */
class fork_process_source : public process_source {
 private:
  // The name of the program which we should exec() into with libmcmini.so
  // preloaded.
  std::string target_program;  // TODO: Favor std::filesystem::path if C++17
                               // is eventually supported
                               // Alternatively, have McMini conditionally
                               // compile a std::filesystem::path e.g.

 public:
  fork_process_source(std::string target_program)
      : target_program(std::move(target_program)) {}

  std::unique_ptr<process> make_new_process() override;
};

}  // namespace mcmini::real_world