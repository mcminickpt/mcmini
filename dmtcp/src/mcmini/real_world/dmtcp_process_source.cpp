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

using namespace real_world;

dmtcp_process_source::dmtcp_process_source(const std::string& ckpt_file)
    : ckpt_file(ckpt_file) {}

void dmtcp_process_source::preload_template_for_state_consumption() {
  if (!has_template_process_alive()) make_new_template_process();
}

void dmtcp_process_source::make_new_template_process() {
  target dmtcp_restart("dmtcp_restart", {this->ckpt_file});

  // Reset first. If an exception is raised in subsequent steps, we don't want
  // to erroneously think that there is a template process when indeed there
  // isn't one.
  this->template_pid = dmtcp_process_source::no_template;

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    throw std::runtime_error("Failed to open pipe(2): " +
                             std::string(strerror(errno)));
  }

  errno = 0;
  pid_t const child_pid = fork();
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

    setenv("MCMINI_DMTCP_RESTART", "1", 1);
    dmtcp_restart.execvp(false);
    unsetenv("MCMINI_DMTCP_RESTART");

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
    this->template_pid = child_pid;
  }
}

std::unique_ptr<process> dmtcp_process_source::make_new_process() {
  // TODO: Implement later
  return nullptr;
}

dmtcp_process_source::~dmtcp_process_source() {
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
