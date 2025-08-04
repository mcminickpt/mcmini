#include <stdio.h>
#include <sys/personality.h>

#include "mcmini/mcmini.h"

void mc_prevent_addr_randomization(void) {
  if (personality(ADDR_NO_RANDOMIZE) == -1) {
    perror("personality");
    mc_exit(EXIT_FAILURE);
  }
}

void mc_install_sig_handlers(void) {
  // TODO: Install SIGCHLD handler to pass to the parent process.
  // struct sigaction action;
  // action.sa_handler = &;
  // sigemptyset(&action.sa_mask);
  // sigaction(SIGINT, &action, NULL);
}
