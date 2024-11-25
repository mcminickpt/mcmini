#include "mcmini/real_world/target.hpp"

#include <fcntl.h>
#include <libgen.h>
#include <string.h>
#include <sys/personality.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <csignal>
#include <cstring>
#include <fstream>
#include <iostream>

#include "mcmini/real_world/dmtcp_target.hpp"
#include "mcmini/real_world/process/dmtcp_coordinator.hpp"
#include "mcmini/real_world/process_source.hpp"

using namespace real_world;

void target::execvp() const {
  // `const_cast<>` is needed to call the C-functions here. A new/delete
  // or malloc/free _could be_ needed, we'd need to check the man page. As
  // long as the char * is not actually modified, this is OK and the best way
  // to interface with the C library routines
  std::vector<char *> args;
  args.reserve(this->target_program_args.size());
  args.push_back(const_cast<char *>(this->target_program.c_str()));
  for (const std::string &arg : this->target_program_args)
    args.push_back(const_cast<char *>(arg.c_str()));
  args.push_back(NULL);

  // Ensures that addresses in the template process remain "stable"
  personality(ADDR_NO_RANDOMIZE);

  // Ensures that the template process is sent a SIGTERM if THIS THREAD ever
  // exits. Since McMini is currently single-threaded, this is equivalent to
  // saying if McMini ever exits. Note that this `prctl(2)` persists across
  // `execvp(2)`.
  prctl(PR_SET_PDEATHSIG, SIGTERM);

  // Ensures that the child will accept the reception of all signals (see
  // `install_process_wide_signal_handlers()` where we explicitly block the
  // reception of signals from the main thread i.e. this thread which is
  // calling `fork()`)
  //
  // The man page for `sigprocmask(2)` reads:
  //
  // "A child created via fork(2) inherits a copy of its parent's signal mask;
  // the signal mask is preserved across execve(2)."
  //
  // hence why we clear it about before `execvp()`
  sigset_t empty_set;
  sigemptyset(&empty_set);
  sigprocmask(SIG_SETMASK, &empty_set, NULL);
  ::execvp(this->target_program.c_str(), args.data());
}

pid_t target::launch_dont_wait() {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    throw std::runtime_error("Failed to open pipe(2): " +
                             std::string(strerror(errno)));
  }
  errno = 0;
  const pid_t child_pid = ::fork();
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

    for (const auto &env_pair : this->setenv_vars)
      setenv(env_pair.first.c_str(), env_pair.second.c_str(), 1);

    for (const auto &unsetenv_var : this->unsetenv_vars)
      unsetenv(unsetenv_var.c_str());

    execvp();

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
          "Failed to create a new process of '" + this->target_program + "'" +
          " (execvp(2) failed with error code '" + std::to_string(errno) +
          "'):" + std::string(strerror(err)));
    }
    close(pipefd[0]);

    // *******************
    // Parent process case
    // *******************
    return child_pid;
  }
}

void target::launch_and_wait() {
  int status = 0;
  pid_t child = launch_dont_wait();
  if (waitpid(child, &status, 0) == -1) {
    throw process::execution_error(
        "Failed to create a cleanup zombied child process (waitpid(2) "
        "returned -1): " +
        std::string(strerror(errno)));
  }
  if (WIFEXITED(status)) {
    std::cerr << "Exited with status" << WEXITSTATUS(status);
  } else if (WIFSIGNALED(status)) {
    throw process::execution_error(
        "The child process was signaled (received :" +
        std::to_string(WTERMSIG(status)) + ")");
  } else {
    throw process::execution_error("The child process exited abnormally");
  }
}

pid_t dmtcp_target::launch_dont_wait() {
  // From the DMTCP doc:
  //"""
  // DMTCP preserves the original pre-checkpoint environment variables when
  // it restarts. Usually, this is the desired behavior. In a few cases,
  // one wishes to update the environment variables based on their values
  // within the parent process of the restarted process. These updates
  // can be specified in a file:
  //   dmtcp_env.txt
  // The file dmtcp_env.txt must be in the same checkpoint directory as
  // the checkpoint image files ( ckpt_*.dmtcp ).
  //"""
  if (propagate_env_through_dmtcp &&
      ((this->unsetenv_vars != this->unsetenv_file_vars) ||
       (this->setenv_vars != this->setenv_file_vars))) {
    std::string ckpt_dir;
    size_t pos = this->ckpt_file_path.find_last_of("/");
    if (std::string::npos != pos) {
      ckpt_dir = this->ckpt_file_path.substr(0, pos);
    } else {
      // Fallback to the current directory if there are no slashes
      ckpt_dir = ".";
    }
    std::string dmtcp_env_file_path = std::move(ckpt_dir) + "/dmtcp_env.txt";
    this->dmtcp_env_file = std::fstream(
        dmtcp_env_file_path, std::ios_base::trunc | std::fstream::out);

    // Write the environment vars out to the file. The format is "VAR=VALUE"
    // according to the docs:
    // """
    // Provide 'name=value' pairs in a format similar to that of putenv().
    // The 'name=value' will be added to the environment of the restarted
    // process.
    //
    // A 'name' with no '=' indicates to unset 'name' in the restarted
    // process.
    // """
    this->dmtcp_env_file << "# This file was autogenerated by `mcmini`\n";
    for (const auto &pair : this->setenv_vars)
      this->dmtcp_env_file << pair.first << "="
                           << "\"" << pair.second << "\"\n";
    for (const auto &name : this->unsetenv_vars)
      this->dmtcp_env_file << name << "\n";
    this->dmtcp_env_file.flush();

    // Cache the current variables just written to the output.
    this->setenv_file_vars = this->setenv_vars;
    this->unsetenv_file_vars = this->unsetenv_vars;
  }
  return target::launch_dont_wait();
}

dmtcp_coordinator::dmtcp_coordinator()
    : dmtcp_target("dmtcp_coordinator",
                   {"--daemon", "--port", "0", "--port-file",
                    "mcmini_dmtcp_coordinator_port"}) {
  this->propagate_env_through_dmtcp = false;
}

void dmtcp_coordinator::launch_and_wait() {
  dmtcp_target::launch_and_wait();
  // Returning from the call to `fork()` does _NOT_ guarantee
  // that the forked child process has completed its `execvp()`.
  // We need to wait for the coordinator to be ready
  //
  // TODO: Remove the busy wait here
  std::ifstream dmtcp_coord_port_stream("mcmini_dmtcp_coordinator_port");
  while (!dmtcp_coord_port_stream.is_open()) {
    usleep(100000);
    dmtcp_coord_port_stream.open("mcmini_dmtcp_coordinator_port");
  }
  dmtcp_coord_port_stream >> this->port;
  unlink("mcmini_dmtcp_coordinator_port");
}
