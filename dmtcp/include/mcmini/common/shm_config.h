#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "mcmini/defines.h"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/template_process.h"

const extern size_t shm_size;
void mc_get_shm_handle_name(char *dst, size_t sz);

struct mcmini_shm_file {
  struct template_process_t tpt;
  runner_mailbox mailboxes[MAX_TOTAL_THREADS_IN_PROGRAM];
};

#ifdef __cplusplus
}
#endif  // extern "C"
