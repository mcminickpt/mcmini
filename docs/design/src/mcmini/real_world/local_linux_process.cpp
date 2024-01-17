#include "mcmini/real_world/process/local_linux_process.hpp"

using namespace mcmini::real_world;

local_linux_process::~local_linux_process() {
  // TODO: Complete implementation
  if (pid <= 0) {
    return;
  }
  kill(pid, SIGUSR1);    /* TODO: React to errors here */
  waitpid(pid, NULL, 0); /* TODO: React to errors here */
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