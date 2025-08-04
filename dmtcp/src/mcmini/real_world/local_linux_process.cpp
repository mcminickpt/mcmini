#include "mcmini/real_world/process/local_linux_process.hpp"

#include <errno.h>
#include <signal.h>

#include <atomic>
#include <cstring>
#include <iostream>

#include "mcmini/common/shm_config.h"
#include "mcmini/log/logger.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/fork_process_source.hpp"
#include "mcmini/real_world/process/resources.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/signal.hpp"

using namespace logging;
using namespace real_world;

logger process_logger("processes");

local_linux_process::local_linux_process(pid_t pid, bool should_wait)
    : pid(pid), should_wait(should_wait) {}

local_linux_process::local_linux_process(local_linux_process &&other)
    : local_linux_process(other.pid) {
  other.pid = -1;
}

local_linux_process &local_linux_process::operator=(
    local_linux_process &&other) {
  this->pid = other.pid;
  other.pid = -1;
  return *this;
}

local_linux_process::~local_linux_process() {
  if (pid <= 0) {
    return;
  }
  if (kill(pid, SIGUSR1) == -1) {
    log_error(process_logger)
        << "Error sending SIGUSR1 to `" << (pid) << "`: " << strerror(errno);
  }
  int status;
  if (should_wait) {
    if (waitpid(pid, &status, 0) == -1) {
      if (errno != ECHILD) {
        log_error(process_logger) << "Error waiting for process (waitpid) `"
                                  << pid << "`: " << strerror(errno);
      } else {
        log_error(process_logger) << "Error: " << strerror(errno);
      }
    }
  }
}

volatile runner_mailbox *local_linux_process::execute_runner(runner_id_t id) {
  shared_memory_region *shm_slice =
      xpc_resources::get_instance().get_rw_region();
  volatile runner_mailbox *rmb =
      &(shm_slice->as_array_of<mcmini_shm_file>()->mailboxes[id]);

  signal_tracker::set_sem((sem_t *)&rmb->model_side_sem);

  // NOTE: At the moment, each process has the entire view of the
  // shared memory region at its disposal. If desired, an extra layer could be
  // added on top which manages allocating slices of a `shared_memory_region`
  // and "allocates" them to different processes. This would look similar to
  // `malloc()/free()`, where the `free()` would be triggered by the destructor
  // of the slice. This is overly complicated at the moment, and we simply
  // restrict the number of proxy processes to one.
  mc_wake_thread(rmb);

  // There is a potential race if the child dies and issues a SIGCHLD
  // just before we call `mc_wait_for_thread()`. This doesn't happen
  // because the signal handler for SIGCHLD will call `mc_wake_scheduler()`
  // which issues a `sem_post(3)` to cancel the `sem_wait(3)` in
  // `mc_wait_for_thread()`. The signal handler also sets `sigchld_set`,
  // allowing us to determine that a SIGCHLD was received.
  //
  // TODO: The template process will also send a SIGCHLD if it dies
  // unexpectedly. Because we don't expect the template process to die, this is
  // OK for now, but should be handled in the future.
  errno = 0;
  signal_tracker::sig_semwait((sem_t *)&rmb->model_side_sem);
  if (signal_tracker::instance().try_consume_signal(SIGCHLD)) {
    // TODO: Get the true failure status from the template process via
    // e.g. shared memory.
    throw process::termination_error(SIGTERM, id,
                                     "Process terminated abnormally.");
    // // TODO: Double check that this
    // // is the correct process that sent
    // // the SIGCHILD using WNOHANG.

    // // `PR_SET_CHILD_SUBREAPER` enables us to wait on
    // // this grandchild.
    // int status;
    // int rc = waitpid(this->pid, &status, 0);
    // if (rc == -1) {
    //   throw process::execution_error(
    //       "Error attempting to determine the failure causing the child
    //       process " "to abnormally exit (or possibly an internal error of
    //       McMini)." + std::string(strerror(errno)));
    // } else {
    //   if (WIFEXITED(status)) {
    //     int exit_code = WEXITSTATUS(status);
    //     throw process::nonzero_exit_code_error(
    //         exit_code, "Process terminated with a non-zero exit code.");
    //   } else if (WIFSIGNALED(status)) {
    //     int signo = WTERMSIG(status);
    //     throw process::termination_error(signo,
    //                                      "Process terminated abnormally.");
    //   } else {
    //     throw process::execution_error(
    //         "SIGSTOP/SIGCONT in branch processes is not yet supported.");
    //   }
    // }
  }
  return rmb;
}
