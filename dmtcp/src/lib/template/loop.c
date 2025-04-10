#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dmtcp.h>

#include "mcmini/common/exit.h"
#include "mcmini/common/shm_config.h"
#include "mcmini/defines.h"
#include "mcmini/lib/entry.h"
#include "mcmini/lib/log.h"
#include "mcmini/lib/template.h"
#include "mcmini/spy/checkpointing/record.h"
#include "mcmini/spy/intercept/interception.h"

void mc_prepare_new_child_process(pid_t ppid_before_fork) {
  // IMPORTANT: If the THREAD in the template process ever exits, this will
  // prove problematic as it is when the THREAD which called `fork()` exits that
  // the signal will be delivered to this process (and not when the process as a
  // whole exits)
  if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1) {
    perror("prctl");
    libc_abort();
  }

  // HANDLE RACE CONDITION! It's possible that the parent exited BEFORE the call
  // to `prctl()` above. To test if this is the case, we check whether the
  // current parent process id differs from the process id of the parent which
  // fork()-ed this process. If they don't match, we know that the parent exited
  // before prctl()
  if (dmtcp_real_to_virtual_pid(getppid()) != ppid_before_fork) mc_exit(EXIT_FAILURE);

  // This is important to handle the case when the
  // main thread exits `in main()`; in that case, we
  // keep the process alive to allow the model checker to
  // continue working
  //
  // NOTE: `atexit()`-handlers can be invoked when a dynamic
  // library is unloaded.
  atexit(&mc_exit_main_thread_in_child);

  // TODO: It's not clear if we need to do this or not...
  //
  // Remove all signal handlers or the child
  // struct sigaction action;
  // action.sa_handler = SegvfaultHandler;
  // sigemptyset(&action.sa_mask);
  // sigaction(SIGSEGV, &action, NULL);
}
