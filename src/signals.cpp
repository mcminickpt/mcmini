#include "mcmini/signals.h"
#include "mcmini/mcmini_private.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
sigsethandler(int sig, void (*handler)(int))
{
  struct sigaction action;
  action.sa_flags   = 0;
  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  return sigaction(sig, &action, NULL);
}

int
sigremovehandler(int sig)
{
  return sigsethandler(sig, SIG_DFL);
}

int
install_sighandles_for_trace()
{
  int rc = 0;

  // Remove handlers set for the scheduler process.
  // These are intended only for the scheduler
  rc |= sigremovehandler(SIGUSR1);
  rc |= sigremovehandler(SIGINT);
  rc |= sigremovehandler(SIGCHLD);
  rc |= sigsethandler(SIGUSR1, &sigusr1_handler_trace);
  return rc;
}

void
sigusr1_handler_trace(int sig)
{
  // Sent by the parent to the trace to force the trace to exit
  _exit(EXIT_SUCCESS);
}

int
install_sighandles_for_scheduler()
{
  int rc = sigsethandler(SIGUSR1, &sigusr1_handler_scheduler);
  rc |= sigsethandler(SIGINT, &sigint_handler_scheduler);

  struct sigaction action;
  action.sa_flags     = SA_SIGINFO;
  action.sa_sigaction = &sigchld_handler_scheduler;
  sigemptyset(&action.sa_mask);
  rc |= sigaction(SIGCHLD, &action, NULL);

  return rc;
}

void
sigusr1_handler_scheduler(int sig)
{
  char msg[] = "*** FATAL ERROR: Trace failed unexpectedly ***\n";
  write(STDERR_FILENO, msg, sizeof(msg));
  mc_stop_model_checking(EXIT_FAILURE);
}

void
sigint_handler_scheduler(int sig)
{
  char msg[] = "\nmcmini: interrupted\n";
  write(STDERR_FILENO, msg, sizeof(msg));
  mc_stop_model_checking(EXIT_SUCCESS);
}

void
sigchld_handler_scheduler(int sig, siginfo_t *info, void *unused)
{
  // This is the normal case: a child exited normally (i.e. was sent a
  // SIGUSR1 and was *explicitly* killed by the scheduler since we
  // intercept calls to exit(2)) so this is not an error
  if (info->si_code == CLD_EXITED) { return; }

  // Another normal case: SIGINT should not be a cause for alarm
  // from the child
  if (info->si_code == CLD_KILLED && info->si_status == SIGINT) {
    return;
  }

  // FIXME: Most of the function calls made below are not async-signal
  // safe. We need to fix this in the future to improve McMini's
  // robustness/correctness
  char msg[512];
  int len =
    snprintf(msg, sizeof(msg), "Trace %lu stopped early!\n", traceId);
  msg[len] = '\0';

  write(STDERR_FILENO, msg, len);
  fsync(STDERR_FILENO);

  // Write the trace contents out
  programState->printTransitionStack();
  programState->printNextTransitions();
  mc_stop_model_checking(EXIT_FAILURE);
}