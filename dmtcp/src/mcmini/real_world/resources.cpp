#include "mcmini/real_world/process/resources.hpp"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "mcmini/common/shm_config.h"
#include "mcmini/misc/extensions/unique_ptr.hpp"

using namespace real_world;
using namespace extensions;

xpc_resources::xpc_resources() {
  const std::string shm_file_name = "/mcmini-" + std::string(getenv("USER")) +
                                    "-" + std::to_string((long)getpid());
  this->rw_region = make_unique<shared_memory_region>(shm_file_name, shm_size);

  // TODO: This should be a configurable parameter perhaps...
  volatile mcmini_shm_file* shm_file = rw_region->as<mcmini_shm_file>();
  const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
  for (int i = 0; i < max_total_threads; i++)
    mc_runner_mailbox_init(&shm_file->mailboxes[i]);
}

void xpc_resources::reset_binary_semaphores_for_new_process() {
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
