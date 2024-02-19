#include "mcmini/real_world/process/local_linux_process.hpp"

#include <errno.h>
#include <sys/wait.h>

#include <cstring>
#include <iostream>

using namespace real_world;

local_linux_process::~local_linux_process() {
  if (pid <= 0) {
    return;
  }
  if (kill(pid, SIGUSR1) == -1) {
    std::cerr << "Error sending SIGUSR1 to process " << pid << ": "
              << strerror(errno) << std::endl;
  }

  int status;
  if (waitpid(pid, &status, 0) == -1) {
    std::cerr << "Error waiting for process " << pid << ": " << strerror(errno)
              << std::endl;
  } else if (!WIFEXITED(status)) {
    std::cerr << "Process " << pid << " did not exit normally." << std::endl;
    if (WIFSIGNALED(status)) {
      std::cerr << "Process " << pid << " was terminated by signal "
                << WTERMSIG(status) << std::endl;
    }
  }
}

std::istream &local_linux_process::execute_runner(
    runner_id_t mcmini_runner_id) {
  /* TODO: sem_wait + sem_post pair */

  /* TODO: Reset the std::streambuf wrapper to point to the beginning of the
   * read end */

  /* TODO: Return the wrapper (obviously not as a dangling reference)*/
  std::istream a(nullptr);
  return a;
}