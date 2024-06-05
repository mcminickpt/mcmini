#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <semaphore.h>
#include <signal.h>

#define TEMPLATE_FORK_FAILED (-2)  // fork(2) failed in the template

struct template_process_t {
  /// Unused in most cases, but set in the event of an error after
  /// calling a C function in the template process (e.g. after `fork(2)` fails).
  int err;

  /// The process id of the latest `fork()` of the template process, or
  /// `TEMPLATE_FORK_FAILED` in the event that `fork(2)` in the template process
  /// failed.
  pid_t cpid;

  /// If a `siginfo_t` was passed to the template process from its current
  /// child, the template will populate this value.
  siginfo_t sa_cinfo;

  /// A semaphore that the McMini process waits on and that
  /// the template process signals after writing the newly spawned pid to shared
  /// memory.
  sem_t mcmini_process_sem;

  /// A semphore that `libmcmini.so` waits on and that the McMini process
  /// signals when it wants to spawn a new process.
  sem_t libmcmini_sem;
};

#ifdef __cplusplus
}
#endif  // extern "C"
