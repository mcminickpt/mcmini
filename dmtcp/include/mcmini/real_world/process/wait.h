#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/wait.h>

//
struct wait_status {
  pid_t cpid;
};

struct wait_status mc_wait();
struct wait_status mc_waitpid(pid_t, int);

#ifdef __cplusplus
}
#endif  // extern "C"
