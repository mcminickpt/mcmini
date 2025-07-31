#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dmtcp.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include "mcmini/lib/sig.h"
#include "mcmini/common/exit.h"
#include "mcmini/common/shm_config.h"
#include "mcmini/defines.h"
#include "mcmini/lib/entry.h"
#include "mcmini/lib/log.h"
#include "mcmini/lib/template.h"
#include "mcmini/spy/checkpointing/record.h"
#include "mcmini/spy/intercept/interception.h"

void mc_prepare_new_child_process(pid_t template_pid, pid_t model_checker_pid) {
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
  if (mcmini_real_pid(getppid()) != template_pid) mc_exit(EXIT_FAILURE);

  // This is important to handle the case when the
  // main thread exits `in main()`; in that case, we
  // keep the process alive to allow the model checker to
  // continue working
  //
  // NOTE: `atexit()`-handlers can be invoked when a dynamic
  // library is unloaded.
  atexit(&mc_exit_main_thread_in_child);

  // FIXME: Make sure to restore the old signal handler
  // for SIGCHLD replaced in the template process in favor
  // of any SIGCHLD handler previously installed.
  //
  // This is related to a future optimization in which the
  // branch process directly signals the McMini process.
  // The branch process would have to install a custom handler
  // to handle the case where the branch process aborts
  // unexpectedly.
  struct sigaction action;
  action.sa_handler = SIG_DFL;
  sigemptyset(&action.sa_mask);
  sigaction(SIGCHLD, &action, NULL);
}

#define NO_DEFINED_MCMINI_PID (-1)
static sem_t sigchld_sem;
static volatile sig_atomic_t global_model_checker_pid = NO_DEFINED_MCMINI_PID;

void mc_template_receive_sigchld(int sig, siginfo_t *info, void *) {
    assert(global_model_checker_pid != NO_DEFINED_MCMINI_PID);
    printf("signalling!!\n");
    fsync(STDOUT_FILENO);
    int status;
    bool signal_mcmini = false;
    int rc = waitpid(-1, &status, 0);
    if (rc == -1) {
      // Error with waitpid. Signal McMini?
      perror("waitpid");
      return;
    }
    if (WIFEXITED(status)) {
      int exit_code = WEXITSTATUS(status);
      signal_mcmini = exit_code != 0;
    }
    else if (WIFSIGNALED(status)) {
      int signo = WTERMSIG(status);
      signal_mcmini = is_bad_signal(signo);
    }
    if (signal_mcmini) {
      printf("signalling McMini!!\n");
      // TODO: We can use `sigqueue(3)` to pass the exit status of
      // the child to the McMini process
      //
      //  e.g. `sigqueue(global_model_checker_pid, SIG... ...)`
      //
      // Alternatively, the syscall `rt_sigqueueinfo(2)` can be used
      // to deliver the `siginfo_t` directly.
      //
      // See https://man7.org/linux/man-pages/man2/rt_sigqueueinfo.2.html
      kill(global_model_checker_pid, SIGCHLD);
    }
    libpthread_sem_post(&sigchld_sem);
}

void mc_template_process_loop_forever(pid_t (*make_new_process)(void)) {
  volatile struct mcmini_shm_file *shm_file = global_shm_start;
  volatile struct template_process_t *tpt = &shm_file->tpt;
  const pid_t model_checker_pid = getppid();
  const pid_t ppid_before_fork = getpid();
  global_model_checker_pid = model_checker_pid;

  libpthread_sem_init(&sigchld_sem, 0, 0);

  struct sigaction action = {0};
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_sigaction = &mc_template_receive_sigchld;
  action.sa_flags |= SA_NOCLDSTOP;
  sigaction(SIGCHLD, &action, NULL);

  while (1) {
    libpthread_sem_wait_loop((sem_t *)&tpt->libmcmini_sem);
    const pid_t cpid = make_new_process();
    if (cpid == -1) {
      // `fork()` failed
      tpt->err = errno;
      tpt->cpid = TEMPLATE_FORK_FAILED;
    } else if (cpid == 0) {
      // Child case: Simply return and escape into the child process
      mc_prepare_new_child_process(ppid_before_fork, model_checker_pid);
      return;
    }
    // `libmcmini.so` acting as a template process.
    tpt->cpid = cpid;
    libpthread_sem_post((sem_t *)&tpt->mcmini_process_sem);

    log_debug("Waiting for the child `%d` to exit... \n", cpid);
    libpthread_sem_wait_loop(&sigchld_sem);
    log_debug("The child exited! Circling back... %d\n", cpid);
  }
}

void mc_template_thread_loop_forever(void) {
  bool has_transferred_state = false;

  volatile struct mcmini_shm_file *shm_file = global_shm_start;
  volatile struct template_process_t *tpt = &shm_file->tpt;
  const pid_t model_checker_pid = mcmini_real_pid(getppid());
  const pid_t template_pid = mcmini_real_pid(getpid());
  global_model_checker_pid = model_checker_pid;

  // Prior to creating new child processes, the template thread
  // unblocks SIGCHLD and installs a signal handler. When the child
  // process dies, the template delivers a SIGCHLD to the parent process
  // in its place via `kill(2)` (which is async-signal-safe)
  //
  // See the Linux ma page for `sigaction(2)`:
  //
  // """
  // SA_NOCLDSTOP:
  //
  // If signum is SIGCHLD, do not receive notification when child processes stop
  // (i.e., when they receive one  of SIGSTOP,  SIGTSTP,  SIGTTIN, or SIGTTOU)
  // or resume (i.e., they receive SIGCONT) (see wait(2)).  This flag is
  // meaningful only when establishing a handler for SIGCHLD.
  // """
  //
  // NOTE: We could add SIGUSR1 to the `sa_mask`. This would ensure that if
  // the McMini process delivered a SIGUSR1 while the template were inside its
  // SIGCHLD handler, the McMini process would still receive the (forwarded)
  // SIGCHLD. It's unclear if this would be useful at the time of writing.
  //
  // NOTE: We need to wait until the `SIGCHLD` is delivered before we continue
  // execution. Although we could also use `sigwait()`, the disadvantage with using
  // `sigwait()` is that the SA_NOCLDSTP flag has no equivalent with `sigwait()`.
  // Instead, we rely on the fact that `sem_post(3)` is async-signal-safe
  //
  // TODO: A future optimization could be to ignore the SIGCHLD altogether in
  // the template process.
  //
  libpthread_sem_init(&sigchld_sem, 0, 0);

  struct sigaction action = {0};
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_sigaction = &mc_template_receive_sigchld;
  action.sa_flags |= SA_NOCLDSTOP;
  sigaction(SIGCHLD, &action, NULL);

  sigset_t sigchld;
  sigemptyset(&sigchld);
  sigaddset(&sigchld, SIGCHLD);
  pthread_sigmask(SIG_UNBLOCK, &sigchld, NULL);

  while (1) {
    log_debug("Waiting for `mcmini` to signal a fork");
    libpthread_sem_wait((sem_t *)&tpt->libmcmini_sem);
    log_debug("`mcmini` signaled a fork!");

    const pid_t cpid = multithreaded_fork();
    if (cpid == -1) {
      // `multithreaded_fork()` failed
      log_debug("The template process failed to create a new child%d\n");
      tpt->err = errno;
      tpt->cpid = TEMPLATE_FORK_FAILED;
    } else if (cpid == 0) {
      // Child case: Simply return and escape into the child process.
      mc_prepare_new_child_process(template_pid, model_checker_pid);
      return;
    }
    else {
      // Successful parent case
      log_debug("The template process created child with pid %d\n", cpid);
      tpt->cpid = cpid;
    }
    libpthread_sem_post((sem_t *)&tpt->mcmini_process_sem);

    if (!has_transferred_state) {
      has_transferred_state = true;
      unsetenv("MCMINI_NEEDS_STATE");
    }

    log_debug("Waiting for the child `%d` to exit... \n", cpid);
    libpthread_sem_wait_loop(&sigchld_sem);
    log_debug("The child exited! Circling back... %d\n", cpid);
  }
}
