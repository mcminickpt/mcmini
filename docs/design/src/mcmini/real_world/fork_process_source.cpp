#include "mcmini/real_world/process/fork_process_source.hpp"

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <mutex>

#include "mcmini/common/shm_config.h"
#include "mcmini/defines.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/local_linux_process.hpp"

using namespace real_world;
using namespace extensions;

std::unique_ptr<shared_memory_region> fork_process_source::rw_region = nullptr;

void fork_process_source::initialize_shared_memory() {
  const std::string shm_file_name = "/mcmini-" + std::string(getenv("USER")) +
                                    "-" + std::to_string((long)getpid());

  rw_region = make_unique<shared_memory_region>(shm_file_name, shm_size);

  volatile runner_mailbox* mbp = rw_region->as_stream_of<runner_mailbox>();

  // TODO: This should be a configurable parameter perhaps...
  const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
  for (int i = 0; i < max_total_threads; i++) {
    mc_runner_mailbox_init(mbp + i);
  }
}

fork_process_source::fork_process_source(std::string target_program)
    : target_program(std::move(target_program)) {
  static std::once_flag shm_once_flag;
  std::call_once(shm_once_flag, initialize_shared_memory);
}

std::unique_ptr<process> fork_process_source::make_new_process() {
  // const_cast<> is needed to call the C-functions here. A new/delete
  // or malloc/free _could be_ needed, we'd need to check the man page. As long
  // as the char * is not actually modified, this is OK and the best way
  // to interface with the C library routines
  setup_ld_preload();

  {
    // Reinitialize the region for the new process, as the contents of the
    // memory are dirtied from the last process which used the same memory and
    // exited arbitrarily (i.e. in such a way as to leave data in the shared
    // memory).
    //
    // INVARIANT: Only one `local_linux_process` is in existence at any given
    // time.
    volatile runner_mailbox* mbp = rw_region->as_stream_of<runner_mailbox>();
    const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
    for (int i = 0; i < max_total_threads; i++) {
      mc_runner_mailbox_destroy(mbp + i);
      mc_runner_mailbox_init(mbp + i);
    }
  }

  errno = 0;
  pid_t child_pid = fork();
  if (child_pid == -1) {
    throw process_source::process_creation_exception(
        "Failed to create a new process (fork(2) failed): " +
        std::string(strerror(errno)));
  } else if (child_pid == 0) {
    // TODO: Add additional arguments here if needed
    char* args[] = {const_cast<char*>(this->target_program.c_str()), NULL};
    std::cerr << "About to exec with libmcmini.so loaded! Attempting to run "
              << this->target_program.c_str() << std::endl;
    execvp(this->target_program.c_str(), args);
    perror("execvp");
    std::exit(EXIT_FAILURE);
  }
  return extensions::make_unique<local_linux_process>(child_pid, *rw_region);
}

void fork_process_source::setup_ld_preload() {
  char buf[1000];
  buf[sizeof(buf) - 1] = '\0';
  snprintf(buf, sizeof buf, "%s:%s/libmcmini.so",
           (getenv("LD_PRELOAD") ? getenv("LD_PRELOAD") : ""),
           dirname(const_cast<char*>(this->target_program.c_str())));
  setenv("LD_PRELOAD", buf, 1);
}