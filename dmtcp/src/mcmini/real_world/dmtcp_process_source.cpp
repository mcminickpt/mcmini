#include "mcmini/real_world/process/dmtcp_process_source.hpp"

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <cassert>

#include "mcmini/common/shm_config.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/process/local_linux_process.hpp"
#include "mcmini/real_world/process/resources.hpp"

using namespace real_world;
using namespace extensions;

dmtcp_process_source::dmtcp_process_source(const std::string& ckpt_file)
    : ckpt_file(ckpt_file) {}

pid_t dmtcp_process_source::make_new_branch() {
  target dmtcp_restart("dmtcp_restart", {this->ckpt_file});

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    throw std::runtime_error("Failed to open pipe(2): " +
                             std::string(strerror(errno)));
  }

  errno = 0;
  const pid_t child_pid = fork();
  if (child_pid == -1) {
    // fork(2) failed
    throw process_source::process_creation_exception(
        "Failed to create a new process (fork(2) failed): " +
        std::string(strerror(errno)));
  }
  if (child_pid == 0) {
    // ******************
    // Child process case
    // ******************
    close(pipefd[0]);
    fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

    setenv("MCMINI_MULTIPLE_RESTARTS", "1", 1);
    dmtcp_restart.execvp(false);
    unsetenv("MCMINI_MULTIPLE_RESTARTS");

    // If `execvp()` fails, we signal the error to the parent process by writing
    // into the pipe.
    int err = errno;
    write(pipefd[1], &err, sizeof(err));
    close(pipefd[1]);

    // @note: We invoke `quick_exit()` here to ensure that C++ static
    // objects are NOT destroyed. `std::exit()` will invoke the destructors
    // of such static objects, among other cleanup. This is only intended to
    // happen exactly once however; bad things likely would happen to a program
    // which called the destructor on an object that already cleaned up its
    // resources.
    //
    // We must remember that this child is in a completely separate process with
    // a completely separate address space, but the shared resources that the
    // McMini process holds onto will also (inadvertantly) be shared with the
    // child. We want the resources to be destroyed in the MCMINI process, NOT
    // this (failed) child fork(). To get C++ to play nicely, this is how we do
    // it.
    std::quick_exit(EXIT_FAILURE);
    // ******************
    // Child process case
    // ******************

    // Should never be reached --> implies quick_exit returned
    std::abort();
    return child_pid;
  } else {
    // *******************
    // Parent process case
    // *******************
    close(pipefd[1]);  // Close write end

    int err = 0;
    if (read(pipefd[0], &err, sizeof(err)) > 0) {
      // waitpid() ensures that the child's resources are properly reacquired.
      if (waitpid(child_pid, nullptr, 0) == -1) {
        throw process_source::process_creation_exception(
            "Failed to create a cleanup zombied child process (waitpid(2) "
            "returned -1): " +
            std::string(strerror(errno)));
      }
      throw process_source::process_creation_exception(
          "Failed to create a new process from the checkpoint image '" +
          this->ckpt_file + "'" + " (execvp(2) failed with error code '" +
          std::to_string(errno) + "'):" + std::string(strerror(err)));
    }
    close(pipefd[0]);

    // *******************
    // Parent process case
    // *******************
    return child_pid;
  }
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
  return extensions::make_unique<local_linux_process>(target_branch_pid, *rw_region);
}
