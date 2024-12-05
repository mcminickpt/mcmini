#pragma once

#include <string>
#include <vector>

#include "mcmini/defines.h"
#include "mcmini/forwards.hpp"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/real_world/target.hpp"

namespace real_world {

/**
 * @brief A factory which produces `real_world::local_linux_process` by
 * `fork()`-ing the templace process process named `target` with
 * `libmcmini.so` preloaded.
 *
 * A `fork_process_source` is responsible for creating new processes by forking
 * and `exec()`-ing into a template process and then repeatedly forking
 * the new process to create new processes. The processes that are vended are
 * duplicates of the template process.
 */
class fork_process_source : public process_source {
 protected:
  std::unique_ptr<target> template_program;
  std::unique_ptr<process_handle> template_process_handle;

  void make_new_template_process();
  bool has_template_process_alive() const {
    return template_process_handle != nullptr;
  }
  friend local_linux_process;

 public:
  fork_process_source() = default;
  fork_process_source(real_world::target&&);
  fork_process_source(const real_world::target&);
  std::unique_ptr<process> make_new_process() override;
};

}  // namespace real_world
