#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <semaphore.h>

#define TEMPLATE_FORK_FAILED (-2)  // fork(2) failed in the template

struct template_process_t {

  // Unused in most cases, but set in the event of an error after
  // calling a C function in the template process
  int err;

  // The current process id of the child process currently under control of this
  // template process.
  pid_t cpid;

  // A semaphore that the McMini process waits on and that
  // the template process signals after writing the newly spawned pid to shared
  // memory.
  sem_t mcmini_process_sem;

  // A semphore that `libmcmini.so` waits on and that the McMini process signals
  // when it wants to spawn a new process.
  sem_t libmcmini_sem;
};

#ifdef __cplusplus
}
#endif  // extern "C"