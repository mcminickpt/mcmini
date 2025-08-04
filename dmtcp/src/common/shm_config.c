#include "mcmini/common/shm_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "mcmini/real_world/mailbox/runner_mailbox.h"

const size_t shm_size = sizeof(struct mcmini_shm_file);

void mc_get_shm_handle_name(char *dst, size_t sz) {
  snprintf(dst, sz, "/mcmini-%s-%lu", getenv("USER"), (long)getppid());
  dst[sz - 1] = '\0';
}

// Define the global variable and initialize the mutex
volatile void* global_shm_start = NULL;
pthread_mutex_t shm_start_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to set the global shared memory start address
void set_global_shm_start(void* addr) {
    pthread_mutex_lock(&shm_start_mutex);
    global_shm_start = addr;
    pthread_mutex_unlock(&shm_start_mutex);
}

// Function to get the global shared memory start address
volatile void* get_global_shm_start() {
    // Use a thread-local variable to cache the value
    static __thread volatile void* local_shm_start = NULL;

    if (local_shm_start == NULL) {
        local_shm_start = global_shm_start;
    }

    return local_shm_start;
}