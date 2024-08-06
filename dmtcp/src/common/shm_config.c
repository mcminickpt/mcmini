#include "mcmini/common/shm_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "mcmini/real_world/mailbox/runner_mailbox.h"

const size_t shm_size = sizeof(runner_mailbox) * MAX_TOTAL_THREADS_IN_PROGRAM;

void mc_get_shm_handle_name(char *dst, size_t sz) {
  snprintf(dst, sz, "/mcmini-%s-%lu", getenv("USER"), (long)getppid());
  dst[sz - 1] = '\0';
}
