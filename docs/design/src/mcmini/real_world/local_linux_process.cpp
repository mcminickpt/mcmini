#include "mcmini/real_world/process/local_linux_process.hpp"

#include <errno.h>
#include <sys/wait.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <mutex>

#include "mcmini/misc/extensions/unique_ptr.hpp"

using namespace real_world;
using namespace extensions;

std::unique_ptr<shared_memory_region> local_linux_process::rw_region = nullptr;
std::unique_ptr<volatile_mem_stream> local_linux_process::vms = nullptr;

void local_linux_process::initialize_shared_memory() {
  rw_region = make_unique<shared_memory_region>("hello", 100);
  vms = make_unique<volatile_mem_stream>(rw_region.get());
}

local_linux_process::local_linux_process(pid_t pid)
    : pid(pid), rw_region_stream(nullptr) {
  static std::once_flag shm_once_flag;
  std::call_once(shm_once_flag, initialize_shared_memory);
}

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

std::istream& local_linux_process::execute_runner(
    runner_id_t mcmini_runner_id) {
  /* TODO: sem_wait + sem_post pair */

  auto* j = rw_region_stream.rdbuf(vms.get());
  auto* j2 = rw_region_stream.rdbuf(vms.get());
  assert(j2 == vms.get());
  // rw_region_stream.seekg(0);

  if (rw_region_stream.fail()) {
    abort();
  }

  // Simulate this for now: write in some value
  volatile uint32_t* v = rw_region->as_stream_of<uint32_t>();
  std::memset((void*)v, 10, rw_region->size());

  return rw_region_stream;
}