#include "mcmini/real_world/process/resources.hpp"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "mcmini/common/shm_config.h"
#include "mcmini/log/logger.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"

using namespace logging;
using namespace real_world;
using namespace extensions;

logger xpc_logger("xpc");

xpc_resources::xpc_resources() {
  const std::string shm_file_name = "/mcmini-" + std::string(getenv("USER")) +
                                    "-" + std::to_string((long)getpid());
  log_debug(xpc_logger) << "Creating shared memory file `" << shm_file_name
                        << "`";
  this->rw_region = make_unique<shared_memory_region>(shm_file_name, shm_size);

  volatile mcmini_shm_file* shm_file = rw_region->as<mcmini_shm_file>();
  volatile template_process_t* tstruct = &shm_file->tpt;

  log_debug(xpc_logger) << "Initializing shared memory semaphores";
  sem_init((sem_t*)&tstruct->mcmini_process_sem, SEM_FLAG_SHARED, 0);
  sem_init((sem_t*)&tstruct->libmcmini_sem, SEM_FLAG_SHARED, 0);

  // TODO: The `MAX_TOTAL_THREADS_IN_PROGRAM` should be a configurable
  // parameter...
  const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
  for (int i = 0; i < max_total_threads; i++)
    mc_runner_mailbox_init(&shm_file->mailboxes[i]);
  log_debug(xpc_logger) << "xpc resource initialization complete";
}

void xpc_resources::reset_binary_semaphores_for_new_branch() {
  // Reinitialize the region for the new process, as the contents of the
  // memory are dirtied from the last process which used the same memory and
  // exited arbitrarily (i.e. in such a way as to leave data in the shared
  // memory).
  //
  // INVARIANT: Only one `local_linux_process` is in existence at any given
  // time.
  log_very_verbose(xpc_logger) << "Destroying shared memory semaphores";
  volatile runner_mailbox* mbp = (rw_region->as<mcmini_shm_file>()->mailboxes);
  const int max_total_threads = MAX_TOTAL_THREADS_IN_PROGRAM;
  for (int i = 0; i < max_total_threads; i++) {
    mc_runner_mailbox_destroy(mbp + i);
    mc_runner_mailbox_init(mbp + i);
  }
  log_very_verbose(xpc_logger) << "Shared memory semaphores reinitialized";
}
