#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

#include "mcmini/defines.h"
#include "mcmini/real_world/mailbox/runner_mailbox.h"
#include "mcmini/real_world/process/template_process.h"
#include "mcmini/spy/checkpointing/rec_list.h"

const extern size_t shm_size;
void mc_get_shm_handle_name(char *dst, size_t sz);

// Declare the global variable and the mutex
extern volatile void* global_shm_start;
extern pthread_mutex_t shm_start_mutex;

// Function declarations
void set_global_shm_start(void* addr);
volatile void* get_global_shm_start();

struct mcmini_shm_file {
  struct template_process_t tpt;
  struct rec_list rec_list;
  runner_mailbox mailboxes[MAX_TOTAL_THREADS_IN_PROGRAM];
};

#ifdef __cplusplus
}
#endif  // extern "C"
