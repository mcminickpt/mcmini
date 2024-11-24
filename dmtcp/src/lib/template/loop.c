#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mcmini/common/shm_config.h"
#include "mcmini/defines.h"
#include "mcmini/lib/entry.h"
#include "mcmini/lib/template.h"
#include "mcmini/spy/checkpointing/record.h"

void mc_prepare_new_child_process(pid_t ppid_before_fork) {
  // IMPORTANT: If the THREAD in the template process ever exits, this will
  // prove problematic as it is when the THREAD which called `fork()` exits that
  // the signal will be delivered to this process (and not when the process as a
  // whole exits)
  if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1) {
    perror("prctl");
    abort();
  }

  // HANDLE RACE CONDITION! It's possible that the parent exited BEFORE the call
  // to `prctl()` above. To test if this is the case, we check whether the
  // current parent process id differs from the process id of the parent which
  // fork()-ed this process. If they don't match, we know that the parent exited
  // before prctl()
  if (getppid() != ppid_before_fork) exit(EXIT_FAILURE);

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
  //   struct sigaction action;
  //   action.sa_handler = SIG_DFL;
  //   sigemptyset(&action.sa_mask);
  //   sigaction(SIGINT, &action, NULL);
  //   sigaction(SIGCHLD, &action, NULL);
}

void mc_template_process_loop_forever(pid_t (*make_new_process)(void)) {
  volatile struct mcmini_shm_file *shm_file = global_shm_start;
  volatile struct template_process_t *tpt = &shm_file->tpt;
  while (1) {
    // IMPORTANT: Before creating any more children, ensure that the previous
    // one has definitely stopped execution.
    //
    // Why is this needed? When `mcmini` wants to explore a new branch of the
    // state space, it kills the old process using `kill(2)` but does _not_ wait
    // for that process to exit after sending kill necessarily (i.e. by calling
    // `waitpid()`).
    //
    // Calling `wait()` avoids the following race:
    //
    // 1. `mcmini` sends `kill(3)` to the branch `B` (the child of this
    // template) but the branch process `B` has not received the signal yet
    // 2. `mcmini` continues execution and decides it wants to explore a new
    // branch, so it signals the template process (this process) to create a new
    // branch `B'`.
    // 3. `B'` uses the same mode of communication as the
    wait(NULL);
    sem_wait((sem_t *)&tpt->libmcmini_sem);
    const pid_t ppid_before_fork = getpid();
    const pid_t cpid = make_new_process();
    if (cpid == -1) {
      // `fork()` failed
      tpt->err = errno;
      tpt->cpid = TEMPLATE_FORK_FAILED;
    } else if (cpid == 0) {
      // Child case: Simply return and escape into the child process.
      mc_prepare_new_child_process(ppid_before_fork);
      return;
    }
    // `libmcmini.so` acting as a template process.
    tpt->cpid = cpid;
    sem_post((sem_t *)&tpt->mcmini_process_sem);
  }
}
