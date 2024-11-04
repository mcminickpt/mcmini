#include "mcmini/real_world/process/fork_process_source.hpp"

#include <fcntl.h>
#include <libgen.h>
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
#include "mcmini/real_world/process/resources.hpp"
#include "mcmini/real_world/process/template_process.h"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/signal.hpp"

using namespace real_world;
using namespace extensions;

std::atomic_uint32_t fork_process_source::num_children_in_flight;

fork_process_source::fork_process_source(
    const real_world::target& target_program)
    : target_program(target_program) {
  // NOTE: According to the man page `dirname(const char *path)` "may modify
  // the contents of `path`...", so we use the storage of the local instead.
  // We don't want to use `std::string` either since it doesn't expect its
  // contents to be modified indirectly
  std::vector<char> target_program_mutable_name(
      this->target_program.name().begin(), this->target_program.name().end());
  char buf[1000];
  buf[sizeof(buf) - 1] = '\0';
  snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
           (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
           dirname(target_program_mutable_name.data()));
  this->target_program.set_env("LD_PRELOAD", buf);
  this->target_program.set_env("MCMINI_TEMPLATE_LOOP", "1");
}

std::unique_ptr<process> fork_process_source::make_new_process() {
  shared_memory_region* rw_region =
      xpc_resources::get_instance().get_rw_region();

  // Assert: only a single child should be in-flight at any point
  if (fork_process_source::num_children_in_flight.load(
          std::memory_order_relaxed) >= 1) {
    throw process_creation_exception(
        "At most one active child process can be in flight at any given time.");
  }

  // 1. Set up phase (LD_PRELOAD, binary sempahores, template process creation)
  xpc_resources::get_instance().reset_binary_semaphores_for_new_branch();
  if (!has_template_process_alive()) {
    make_new_template_process();
  }

  // 2. Check if the current template process has previously exited; if so, it
  // would have delivered a `SIGCHLD` to this process. By default this signal is
  // ignored, but McMini explicitly captures it (see `signal_tracker`).
  if (signal_tracker::instance().try_consume_signal(SIGCHLD)) {
    if (waitpid(this->template_pid, nullptr, 0) == -1) {
      this->template_pid = fork_process_source::no_template;
      throw process_source::process_creation_exception(
          "Failed to create a cleanup zombied child process (waitpid(2) "
          "returned -1): " +
          std::string(strerror(errno)));
    }
    this->template_pid = fork_process_source::no_template;
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
        "The template process (" + std::to_string(template_pid) +
        ") was not synchronized with correctly: " +
        std::string(strerror(errno)));
  }

  if (sem_wait((sem_t*)&tstruct->mcmini_process_sem) != 0) {
    throw process_source::process_creation_exception(
        "The template process (" + std::to_string(template_pid) +
        ") was not synchronized with correctly: " +
        std::string(strerror(errno)));
  }

  if (tstruct->cpid == TEMPLATE_FORK_FAILED) {
    throw process_source::process_creation_exception(
        "The `fork(2)` call in the template process failed unexpectedly "
        "(errno " +
        std::to_string(tstruct->err) + "): " + strerror(tstruct->err));
  }

  fork_process_source::num_children_in_flight.fetch_add(
      1, std::memory_order_relaxed);
  return extensions::make_unique<local_linux_process>(tstruct->cpid);
}

void fork_process_source::make_new_template_process() {
  // Reset first. If an exception is raised in subsequent steps, we don't want
  // to erroneously think that there is a template process when indeed there
  // isn't one.
  this->template_pid = fork_process_source::no_template;
  this->template_pid = this->target_program.fork();
}

fork_process_source::~fork_process_source() {
  if (template_pid <= 0) {
    return;
  }
  if (kill(template_pid, SIGUSR1) == -1) {
    std::cerr << "Error sending SIGUSR1 to process " << template_pid << ": "
              << strerror(errno) << std::endl;
  }
  int status;
  if (waitpid(template_pid, &status, 0) == -1) {
    std::cerr << "Error waiting for process (waitpid) " << template_pid << ": "
              << strerror(errno) << std::endl;
  }
}
