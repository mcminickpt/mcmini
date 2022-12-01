#include "mcmini/signals.h"
#include "mcmini/mcmini_private.h"
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
  rc |= sigsethandler(SIGUSR1, &sigusr1_handler_trace);
  return rc;
}

void
sigusr1_handler_trace(int sig)
{
  // Sent by the parent to the trace to force the trace to exit
  _Exit(0);
}

int
install_sighandles_for_scheduler()
{
  int rc = sigsethandler(SIGUSR1, &sigusr1_handler_scheduler);
  rc |= sigsethandler(SIGINT, &sigint_handler_scheduler);
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