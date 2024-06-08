#pragma once

#include <atomic>
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
 * `fork()`-ing this process followed by an `exec()` into the targeted process
 * located at `target` with `libmcmini.so` preloaded.
 *
 * A `fork_process_source` is responsible for creating new processes by forking
 * a template process, `exec()`-ing into a new one, and then repeatedly forking
 * the new process to create new process sources. The processes that this
 * process source vends are duplicates of the template process
 */
class fork_process_source : public process_source {
 private:
  target target;

  /// @brief The process id of the template process whose libmcmini performs
  /// a `sigwait()` loop ad infinitum.
  ///
  /// This value refers to the process id of the process that is repeatedly
  /// asked to invoke the `fork(2)` system call.
  pid_t template_pid = no_template;
  constexpr static pid_t no_template = -1;

  /// @brief The number of processes that have been created by any process
  /// sources
  ///
  /// @invariant The number of processes that are actively in-flight is
  /// always be <= 1.
  ///
  /// @note the value is atomic in the event that `fork_process_source` is used
  /// by multiple threads. The destructors of `local_linux_process` may
  /// concurrently access this value along with the `make_new_process()` method.
  static std::atomic_uint32_t num_children_in_flight;
  static std::unique_ptr<shared_memory_region> rw_region;

  static void initialize_shared_memory();

  void reset_binary_semaphores_for_new_process();
  void make_new_template_process();
  void template_process_sig_handler();
  bool has_template_process_alive() const { return template_pid != -1; }
  friend local_linux_process;

 public:
  fork_process_source(const real_world::target&);
  ~fork_process_source();
  std::unique_ptr<process> make_new_process() override;
};

}  // namespace real_world
