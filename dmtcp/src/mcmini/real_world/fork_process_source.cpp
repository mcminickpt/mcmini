#include "mcmini/real_world/process/fork_process_source.hpp"

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "mcmini/common/shm_config.h"
#include "mcmini/defines.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process.hpp"
#include "mcmini/real_world/process/local_linux_process.hpp"
#include "mcmini/real_world/process/multithreaded_fork_process_source.hpp"
#include "mcmini/real_world/process/resources.hpp"
#include "mcmini/real_world/process/template_process.h"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/signal.hpp"

using namespace real_world;
using namespace extensions;

fork_process_source::fork_process_source(real_world::target&& tp)
    : fork_process_source(tp) {}

fork_process_source::fork_process_source(
    const real_world::target& target_program)
    : template_program(extensions::make_unique<target>(target_program)) {
  this->template_program->set_preload_libmcmini();
}

multithreaded_fork_process_source::multithreaded_fork_process_source(
    const std::string& ckpt_file) {
  this->coordinator_target.launch_and_wait();
  this->template_program = extensions::make_unique<dmtcp_target>(
      std::string("dmtcp_restart"),
      std::vector<std::string>{
          "--join-coordinator", "--port",
          std::to_string(this->coordinator_target.get_port())},
      ckpt_file);
  this->template_program->set_env("MCMINI_NEEDS_STATE", "1");

  // Possibly a dmtcp program...

  // Here `libmcmini.so` doesn't need to be preloaded: it is assumed that
  // `mcmini` is contained in the checkpoint image that is restored by
  // `dmtcp_restart`. Hence we can omit
  // ```
  // this->target_program.set_preload_libmcmini();
  // ```
}

std::unique_ptr<process> fork_process_source::make_new_process() {
  shared_memory_region* rw_region =
      xpc_resources::get_instance().get_rw_region();

  // 1. Set up phase (LD_PRELOAD, binary sempahores, template process creation)
  xpc_resources::get_instance().reset_binary_semaphores_for_new_branch();
  if (!has_template_process_alive()) make_new_template_process();

  // 2. Check if the current template process has previously exited; if so, it
  // would have delivered a `SIGCHLD` to this process. By default this signal is
  // ignored, but McMini explicitly captures it (see `signal_tracker`).
  if (signal_tracker::instance().try_consume_signal(SIGCHLD)) {
    this->template_process_handle = nullptr;
    throw process_source::process_creation_exception(
        "Failed to create a new process (template process died)");
  }

  // 3. If the current template process is alive, tell it to spawn a new
  // process and then wait for it to successfully call `fork(2)` to tell us
  // about its new child.
  const volatile template_process_t* tstruct =
      &(rw_region->as<mcmini_shm_file>()->tpt);

  if (sem_post((sem_t*)&tstruct->libmcmini_sem) != 0) {
    throw process_source::process_creation_exception(
        "The template process (" +
        std::to_string(this->template_process_handle->get_pid()) +
        ") was not synchronized with correctly: " +
        std::string(strerror(errno)));
  }

  if (sem_wait((sem_t*)&tstruct->mcmini_process_sem) != 0) {
    throw process_source::process_creation_exception(
        "The template process (" +
        std::to_string(this->template_process_handle->get_pid()) +
        ") was not synchronized with correctly: " +
        std::string(strerror(errno)));
  }

  if (tstruct->cpid == TEMPLATE_FORK_FAILED) {
    throw process_source::process_creation_exception(
        "The `fork(2)` call in the template process failed unexpectedly "
        "(errno " +
        std::to_string(tstruct->err) + "): " + strerror(tstruct->err));
  }
  return extensions::make_unique<local_linux_process>(tstruct->cpid, false);
}

void fork_process_source::make_new_template_process() {
  // Reset first. If an exception is raised in subsequent steps, we don't want
  // to erroneously think that there is a template process when indeed there
  // isn't one.
  this->template_program->set_is_template();
  this->template_process_handle = nullptr;
  this->template_process_handle = extensions::make_unique<local_linux_process>(
      this->template_program->launch_dont_wait());
}

void multithreaded_fork_process_source::make_new_template_process() {
  fork_process_source::make_new_template_process();

  // Here we need, in addition, to wait for the template thread
  // to have heard back from all userspace threads before declaing the template
  // process is ready.
  shared_memory_region* rw_region =
      xpc_resources::get_instance().get_rw_region();
  const volatile template_process_t* tstruct =
      &(rw_region->as<mcmini_shm_file>()->tpt);
  if (sem_wait((sem_t*)&tstruct->mcmini_process_sem) != 0) {
    throw process_source::process_creation_exception(
        "The template process (" +
        std::to_string(this->template_process_handle->get_pid()) +
        ") was not synchronized with correctly: " +
        std::string(strerror(errno)));
  }
}
