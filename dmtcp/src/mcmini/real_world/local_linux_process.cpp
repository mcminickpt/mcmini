#include "mcmini/real_world/process/local_linux_process.hpp"

#include <errno.h>
#include <signal.h>

#include <atomic>
#include <cstring>
#include <iostream>

#include "mcmini/common/shm_config.h"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/fork_process_source.hpp"
#include "mcmini/real_world/process/resources.hpp"
#include "mcmini/real_world/shm.hpp"
#include "mcmini/signal.hpp"

using namespace real_world;

local_linux_process::local_linux_process(pid_t pid) : pid(pid) {}

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
    std::cerr << "Error sending SIGUSR1 to `" << (pid)
              << "`: " << strerror(errno);
  }
  int status;
  if (waitpid(pid, &status, 0) == -1)
    std::cerr << "Error waiting for process (waitpid) `" << pid
              << "`: " << strerror(errno);
}

volatile runner_mailbox *local_linux_process::execute_runner(runner_id_t id) {
  shared_memory_region *shm_slice =
      xpc_resources::get_instance().get_rw_region();
  volatile runner_mailbox *rmb =
      &(shm_slice->as_array_of<mcmini_shm_file>()->mailboxes[id]);

  // TODO: As a sanity check, a `waitpid()` to check if the process is still
  // alive is probably warranted. This would prevent a deadlock in _most_ cases.
  // Of course, if the process terminates in between the check and the
  // sem_wait() call, we'd still have deadlock. A happy medium is to call
  // `sem_timedwait()` with a sufficiently long wait value (perhaps 1 second)
  // and poll for existence if we haven't heard from the child in a long time.

  // NOTE: At the moment, each process has the entire view of the
  // shared memory region at its disposal. If desired, an extra layer could be
  // added on top which manages allocating slices of a `shared_memory_region`
  // and "allocates" them to different processes. This would look similar to
  // `malloc()/free()`, where the `free()` would be triggered by the destructor
  // of the slice. This is overly complicated at the moment, and we simply
  // restrict the number of proxy processes to one.
  mc_wake_thread(rmb);

  errno = 0;
  int rc = mc_wait_for_thread(rmb);
  while (rc != 0 && errno == EINTR) {
    rc = mc_wait_for_thread(rmb);
  }
  return rmb;
}
