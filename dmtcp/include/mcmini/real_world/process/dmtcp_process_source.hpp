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
 * @brief A factory which produces `real_world::local_linux_process` by forking
 * a process stored from a checkpoint image via `dmtcp_restart`
 *
 * A `dmtcp_process_source` is responsible for creating new processes by forking
 * and `exec()`-ing into a template process (`dmtcp_restart`) and then
 * repeatedly forking the new process to create new processes. The processes
 * that are vended are duplicates of the template process.
 */
class dmtcp_process_source : public process_source {
 private:
  std::string ckpt_file;

  /// @brief The process id of the template process whose libmcmini performs
  /// a `sem_wait` loop ad infinitum.
  ///
  /// This value refers to the process id of the process that is repeatedly
  /// asked to invoke the `fork(2)` system call.
  pid_t template_pid = no_template;
  constexpr static pid_t no_template = -1;

  // TODO: Extract these into a common spot

  // static void initialize_shared_memory();
  //   void reset_binary_semaphores_for_new_process();
  //   void make_new_template_process();
  //   void template_process_sig_handler();
  //   bool has_template_process_alive() const { return template_pid != -1; }
  //   friend local_linux_process;

 public:
  dmtcp_process_source(const std::string &ckpt_file);
  ~dmtcp_process_source();
  std::unique_ptr<process> make_new_process() override;
};

};  // namespace real_world
