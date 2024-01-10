#include "mcmini/real_world/process/fork_process_source.hpp"

using namespace mcmini::real_world;

std::unique_ptr<process> fork_process_source::make_new_process() {
  this->target_to_send_a_sigusr1_to_repeatedly;

  // Send a signal to that process
  system("kill -p " /* + build the string with target to send" */);

  // TODO: Determine how to create a local_linux_process from this.
  return nullptr;
}