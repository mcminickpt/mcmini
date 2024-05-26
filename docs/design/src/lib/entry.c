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

#include "mcmini/mcmini.h"

volatile void *global_shm_start = NULL;
MCMINI_THREAD_LOCAL runner_id_t tid_self = RID_INVALID;

runner_id_t mc_register_this_thread(void) {
  // NOTE: It is an internal error for more than one thread
  // to be executing this function. If the model checker maintains
  // control over each thread, it will only enable a single
  // thread to execute this function at once. Anything else
  // is undefined behavior
  //
  // NOTE: If `McMini` introduces parallelism into the
  // model-checking process, this would have to be adjusted.
  static runner_id_t tid_next = 0;
  tid_self = tid_next++;
  return tid_self;
}

void mc_allocate_shared_memory_region(void) {
  char dpor[100];
  mc_get_shm_handle_name(dpor, sizeof(dpor));

  int fd = shm_open(dpor, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
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

  /* The parent process will handle shm_unlink() */
  fsync(fd);
  close(fd);
  global_shm_start = gshms;
}

void mc_deallocate_shared_memory_region() {
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

void mc_exit(int status) {
  // The exit() function is intercepted. Calling exit() directly
  // results in a deadlock since the thread calling it will block
  // forever (McMini does not let a process exit() during model
  // checking). Keep this in mind before switching this call to
  // a different exit function
  _Exit(status);
}

void mc_prevent_addr_randomization(void) {
  if (personality(ADDR_NO_RANDOMIZE) == -1) {
    perror("personality");
    mc_exit(EXIT_FAILURE);
  }
}

void mc_install_sig_handlers(void) {
  // struct sigaction action;
  // action.sa_handler = &;
  // sigemptyset(&action.sa_mask);
  // sigaction(SIGINT, &action, NULL);
}

__attribute__((constructor)) void libmcmini_main() {
  mc_prevent_addr_randomization();
  mc_install_sig_handlers();
  mc_register_this_thread();
  mc_load_intercepted_pthread_functions();
  mc_allocate_shared_memory_region();
  atexit(&mc_deallocate_shared_memory_region);

  if (getenv("libmcmini-template-loop") != NULL) {
    mc_template_process_loop_forever();
  }
  thread_await_scheduler_for_thread_start_transition();
}
