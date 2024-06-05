#include "mcmini/common/exit.h"

#include <pthread.h>
#include <stdlib.h>

#include "mcmini/common/exit.h"

void mc_exit(int status) {
  // The exit() function is intercepted. Calling exit() directly
  // results in a deadlock since the thread calling it will block
  // forever (McMini does not let a process exit() during model
  // checking). Keep this in mind before switching this call to
  // a different exit function
  _Exit(status);
}

void mc_exit_mt(int status) {
  static pthread_mutex_t exit_mut = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&exit_mut);
  mc_exit(status);
  pthread_mutex_unlock(&exit_mut);
}
