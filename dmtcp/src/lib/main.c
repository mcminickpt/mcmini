#define _POSIX_C_SOURCE
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/personality.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dmtcp.h"
#include "mcmini/mcmini.h"

volatile void *global_shm_start = NULL;

void mc_allocate_shared_memory_region(void) {
  char dpor[100];
  mc_get_shm_handle_name(dpor, sizeof(dpor));

  int fd = shm_open(dpor, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    if (errno == EACCES) {
      fprintf(stderr, "Shared memory region '%s' not owned by this process\n",
              dpor);
    } else {
      perror("shm_open");
    }
    mc_exit(EXIT_FAILURE);
  }
  int rc = ftruncate(fd, shm_size);
  if (rc == -1) {
    perror("ftruncate");
    mc_exit(EXIT_FAILURE);
  }

  void *gshms = mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (gshms == MAP_FAILED) {
    perror("mmap");
    mc_exit(EXIT_FAILURE);
  }
  fsync(fd);
  close(fd);
  global_shm_start = gshms;
}

void mc_deallocate_shared_memory_region(void) {
  char shm_file_name[100];
  mc_get_shm_handle_name(shm_file_name, sizeof(shm_file_name));
  if (global_shm_start) {
    int rc = munmap((void *)global_shm_start, shm_size);
    if (rc == -1) {
      perror("munmap");
      mc_exit(EXIT_FAILURE);
    }
  }
}

int dmtcp_mcmini_plugin_is_loaded(void) __attribute((weak));
#define dmtcp_mcmini_plugin_is_loaded() \
  (dmtcp_mcmini_plugin_is_loaded? dmtcp_mcmini_plugin_is_loaded() : 0)

__attribute__((constructor)) void libmcmini_main() {
  // In recording mode, the constructor should be ignored and
  // the DMTCP callback should instead be used to determine when
  // `libmcmini.so` wrappers should begin recording.

  if (dmtcp_mcmini_plugin_is_loaded()) {
    // The libmcmini plugin of DMTCP has been loaded.
    // We must be in recording mode.  Don't do model checking yet.
    return;
  }
  mc_prevent_addr_randomization();
  mc_install_sig_handlers();
  mc_register_this_thread();
  mc_allocate_shared_memory_region();
  atexit(&mc_deallocate_shared_memory_region);

  if (getenv("MCMINI_TEMPLATE_LOOP")) {
    libmcmini_mode = TARGET_TEMPLATE;
    mc_template_process_loop_forever();
  }
  thread_await_scheduler_for_thread_start_transition();
}
