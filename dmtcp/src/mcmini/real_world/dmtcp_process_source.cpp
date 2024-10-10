#include "mcmini/real_world/process/dmtcp_process_source.hpp"

#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <iostream>

#include "mcmini/common/shm_config.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/process/local_linux_process.hpp"
#include "mcmini/real_world/process/resources.hpp"

using namespace real_world;
using namespace extensions;

dmtcp_process_source::dmtcp_process_source(const std::string& ckpt_file)
    : ckpt_file(ckpt_file) {
  // FIXME: Returning from the call to `fork()` does _NOT_ guarantee
  // that the forked child process has completed its `execvp()`.
  // To avoid a race in which the coordinator is not yet ready before
  // calling the first `dmtcp_restart`, we need to call `dmtcp_get_coordinator_status()`
  target dmtcp_restart("dmtcp_coordinator", {"--daemon"});
  this->dmtcp_coordinator = local_linux_process(dmtcp_restart.fork());
}

pid_t dmtcp_process_source::make_new_branch() {
  target dmtcp_restart("dmtcp_restart", {"--join-coordinator", this->ckpt_file});
  if (!has_transferred_recorded_objects) {
    dmtcp_restart.set_env("MCMINI_MULTIPLE_RESTARTS", "1");
  }
  return dmtcp_restart.fork();
}

std::unique_ptr<process> dmtcp_process_source::make_new_process() {
  // IMPORTANT: Here, resetting the semaphores for the userspace
  // threads BEFORE creating the new branch process is very important.
  // Otherwise the threads from the restarted checkpoint image would
  // read from the semaphores which the McMini process would overwrite.
  shared_memory_region* rw_region =
      xpc_resources::get_instance().get_rw_region();
  xpc_resources::get_instance().reset_binary_semaphores_for_new_branch();
  pid_t target_branch_pid = make_new_branch();
  this->has_transferred_recorded_objects = true;

  const volatile template_process_t* tstruct =
      &(rw_region->as<mcmini_shm_file>()->tpt);

  if (sem_wait((sem_t*)&tstruct->mcmini_process_sem) != 0) {
    throw process_source::process_creation_exception(
        "The template thread (in process with PID " + std::to_string(target_branch_pid) +
        ") did not synchronize correctly with the McMini process: " +
        std::string(strerror(errno)));
  }

  // Since there is no template process intermediary,
  // the PID of the child process created with `dmtcp_restart`
  // is the PID of the branch. The chain of events is
  //
  // `mcmini process` --> fork()/exec() --> `dmtcp_restart`
  //                  -->  exec() --> `mtcp_restart`
  //
  // The key detail is that `dmtcp_restart` calls `exec()` only.
  // So its PID is preserved.
  assert(tstruct->cpid == target_branch_pid);
  return extensions::make_unique<local_linux_process>(target_branch_pid);
}
