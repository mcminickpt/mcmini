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
#include "mcmini/real_world/process/template_process.h"
#include "mcmini/real_world/process_source.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/signal.hpp"

using namespace real_world;
using namespace extensions;

std::atomic_uint32_t fork_process_source::num_children_in_flight;
std::unique_ptr<shared_memory_region> fork_process_source::rw_region = nullptr;

void fork_process_source::initialize_shared_memory() {
  fork_process_source::num_children_in_flight.store(0,
                                                    std::memory_order_relaxed);
  const std::string shm_file_name = "/mcmini-" + std::string(getenv("USER")) +
                                    "-" + std::to_string((long)getpid());
  rw_region = make_unique<shared_memory_region>(shm_file_name, shm_size);

  volatile runner_mailbox* mbp = rw_region->as_array_of<runner_mailbox>();

  // TODO: This should be a configurable parameter perhaps...
  const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
  for (int i = 0; i < max_total_threads; i++) mc_runner_mailbox_init(mbp + i);
}

fork_process_source::fork_process_source(const real_world::target& target)
    : target(target) {
  static std::once_flag shm_once_flag;
  std::call_once(shm_once_flag, initialize_shared_memory);
}

std::unique_ptr<process> fork_process_source::make_new_process() {
  // Assert: only a single child should be in-flight at any point
  if (fork_process_source::num_children_in_flight.load(
          std::memory_order_relaxed) >= 1) {
    throw process_creation_exception(
        "At most one active child process can be in flight at any given time.");
  }

  // 1. Set up phase (LD_PRELOAD, binary sempahores, template process creation)
  reset_binary_semaphores_for_new_process();
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
  return extensions::make_unique<local_linux_process>(tstruct->cpid,
                                                      *rw_region);
}

void fork_process_source::make_new_template_process() {
  // Reset first. If an exception is raised in subsequent steps, we don't want
  // to erroneously think that there is a template process when indeed there
  // isn't one.
  this->template_pid = fork_process_source::no_template;

  {
    const volatile template_process_t* tstruct =
        rw_region->as<template_process_t>();
    sem_init((sem_t*)&tstruct->mcmini_process_sem, SEM_FLAG_SHARED, 0);
    sem_init((sem_t*)&tstruct->libmcmini_sem, SEM_FLAG_SHARED, 0);
  }

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

    setenv("libmcmini-template-loop", "1", 1);
    target.execvp();
    unsetenv("libmcmini-template-loop");

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
          "Failed to create a new process of '" + this->target.name() + "'" +
          " (execvp(2) failed with error code '" + std::to_string(errno) +
          "'):" + std::string(strerror(err)));
    }
    close(pipefd[0]);

    // *******************
    // Parent process case
    // *******************
    this->template_pid = child_pid;
  }
}

void fork_process_source::reset_binary_semaphores_for_new_process() {
  // Reinitialize the region for the new process, as the contents of the
  // memory are dirtied from the last process which used the same memory and
  // exited arbitrarily (i.e. in such a way as to leave data in the shared
  // memory).
  //
  // INVARIANT: Only one `local_linux_process` is in existence at any given
  // time.
  volatile runner_mailbox* mbp = (rw_region->as<mcmini_shm_file>()->mailboxes);
  const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
  for (int i = 0; i < max_total_threads; i++) {
    mc_runner_mailbox_destroy(mbp + i);
    mc_runner_mailbox_init(mbp + i);
  }
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
    std::cerr << "Error waiting for process (fork) " << template_pid << ": "
              << strerror(errno) << std::endl;
  }
}
