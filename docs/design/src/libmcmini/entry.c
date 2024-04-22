#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include "mcmini/mcmini.h"

volatile void *shm_start = NULL;
MCMINI_THREAD_LOCAL tid_t tid_self = TID_INVALID;

tid_t
mc_register_this_thread()
{
  static tid_t tid_next = 0;
  tid_self = tid_next++;
  return tid_self;
}

void *
mc_allocate_shared_memory_region()
{
  char dpor[100];
  mc_get_shm_handle_name(dpor, sizeof(dpor));

  int fd = shm_open(dpor, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    if (errno == EACCES) {
      fprintf(stderr,
              "Shared memory region '%s' not owned by this process\n",
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

  void *shm_start =
    mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shm_start == MAP_FAILED) {
    perror("mmap");
    mc_exit(EXIT_FAILURE);
  }

  /* The parent process will handle shm_unlink() */
  fsync(fd);
  close(fd);
  return shm_start;
}

void
mc_deallocate_shared_memory_region()
{
  char shm_file_name[100];
  mc_get_shm_handle_name(shm_file_name, sizeof(shm_file_name));
  if (shm_start) {
      int rc = munmap((void*)shm_start, shm_size);
      if (rc == -1) {
        perror("munmap");
        mc_exit(EXIT_FAILURE);
      }
  }
}

void
mc_exit(int status)
{
  // The exit() function is intercepted. Calling exit() directly
  // results in a deadlock since the thread calling it will block
  // forever (McMini does not let a process exit() during model
  // checking). Keep this in mind before switching this call to
  // a different exit function
  _Exit(status);
}

__attribute__((constructor)) void libmcmini_main() {
  mc_register_this_thread();
  mc_load_intercepted_pthread_functions();
  atexit(&mc_deallocate_shared_memory_region);
  shm_start = mc_allocate_shared_memory_region();
  thread_await_scheduler_for_thread_start_transition();
}