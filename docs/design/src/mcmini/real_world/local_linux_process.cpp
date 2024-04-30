#include "mcmini/real_world/process/local_linux_process.hpp"

#include <errno.h>
#include <sys/wait.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <mutex>

#include "mcmini/defines.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"

using namespace real_world;
using namespace extensions;

local_linux_process::local_linux_process(pid_t pid,
                                         shared_memory_region &shm_slice)
    : pid(pid), shm_slice(shm_slice) {}

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

volatile runner_mailbox *local_linux_process::execute_runner(runner_id_t id) {
  volatile runner_mailbox *rmb =
      shm_slice.as_array_of<runner_mailbox>(id, THREAD_SHM_OFFSET);

  // TODO: As a sanity check, a `waitpid()` to check if the process is still
  // alive is probably warranted. This would prevent a deadlock in _most_ cases.
  // Of course, if the process terminates in between the check and the
  // sem_wait() call, we'd still have deadlock. A happy medium is to call
  // `sem_timedwait()` with a sufficiently long wait value (perhaps 1 second)
  // and poll for existence if we haven't heard from the child in a long time.
  mc_wake_thread(rmb);
  mc_wait_for_thread(rmb);
  return rmb;
}