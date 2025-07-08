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

void mc_allocate_shared_memory_region(const char *shm_name) {
  int fd = shm_open(shm_name, O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    if (errno == EACCES) {
      fprintf(stderr, "Shared memory region '%s' not owned by this process\n",
              shm_name);
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
  (dmtcp_mcmini_plugin_is_loaded ? dmtcp_mcmini_plugin_is_loaded() : 0)

__attribute__((constructor)) void libmcmini_main() {
  // In recording mode, the constructor should be ignored and
  // the DMTCP callback should instead be used to determine when
  // `libmcmini.so` wrappers should begin recording.

  // TODD: Why doesn't this work as expected??
  // if (dmtcp_mcmini_plugin_is_loaded()) {
  //   // The libmcmini plugin of DMTCP has been loaded.
  //   // We must be in recording mode.  Don't do model checking yet.
  //   return;
  // }

  if (dmtcp_is_enabled()) {
    // The libmcmini plugin of DMTCP has been loaded.
    // We must be in recording mode.  Don't do model checking yet.
    return;
  }
  // ************************************************
  // STANDARD MODEL CHECKING (NO DEEP DEBUGGING)
  // ************************************************
  mc_prevent_addr_randomization();
  mc_install_sig_handlers();
  mc_register_this_thread();

  char shm_name[100];
  mc_get_shm_handle_name(shm_name, sizeof(shm_name));
  mc_allocate_shared_memory_region(shm_name);
  atexit(&mc_deallocate_shared_memory_region);

  if (getenv("MCMINI_TEMPLATE_LOOP")) {
    set_current_mode(TARGET_TEMPLATE);

    // In classic model checking, verification begins at program start.
    // We can use traditional `fork(3)` instead of `multithreaded_fork()`
    // because at program launch there's only a single thread.
    mc_template_process_loop_forever(&fork);

    // Reaching this point means that we're in the branch: the
    // parent process (aka the template) will never exit
    // the above call to `mc_template_process_loop_forever()`.
  }
  set_current_mode(TARGET_BRANCH);
  thread_await_scheduler();
}
