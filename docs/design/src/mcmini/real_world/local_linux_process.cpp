#include "mcmini/real_world/process/local_linux_process.hpp"

#include <errno.h>
#include <sys/wait.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <mutex>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/runner_mailbox.h"

using namespace real_world;
using namespace extensions;

std::unique_ptr<shared_memory_region> local_linux_process::rw_region = nullptr;

void local_linux_process::initialize_shared_memory() {
  rw_region = make_unique<shared_memory_region>("hello", 100);
}

local_linux_process::local_linux_process(pid_t pid) : pid(pid) {
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

runner_mailbox_stream &local_linux_process::execute_runner(runner_id_t id) {
  volatile runner_mailbox *rmb = rw_region->as_stream_of<runner_mailbox>(id);
  volatile_mem_streambuf runner_mem_stream{&rmb->cnts, sizeof(&rmb->cnts)};

  // TODO: When the `libmcmini.so` portion of the synchronization is complete,
  // we can uncomment this without deadlocking
  // mc_wake_thread(rmb);
  // mc_wait_for_thread(rmb);

  static volatile_mem_streambuf runner_mailbox_bufs[50];
  static runner_mailbox_stream *runner_mailbox_streams[50];

  if (runner_mailbox_streams[id]) {
    delete runner_mailbox_streams[id];
  }
  runner_mailbox_bufs[id] = std::move(runner_mem_stream);
  runner_mailbox_streams[id] =
      new runner_mailbox_stream{&runner_mailbox_bufs[id]};
  return *runner_mailbox_streams[id];
}