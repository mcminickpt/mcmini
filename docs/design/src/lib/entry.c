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

#include "docs/design/include/mcmini/shared_transition.h"
#include "docs/design/include/mcmini/entry.h"

#define MAX_SHARED_MEMORY_ALLOCATION         (4096u)

/* information to be searlized*/
void *shmStart = NULL;
struct SharedTransition* shmTransitionTypeInfo = NULL;
void *shmTransitionData = NULL;
const size_t shmAllocationSize = sizeof(*trace_list) + (sizeof(*shmTransitionTypeInfo)+MAX_SHARED_MEMORY_ALLOCATION) * MAX_TOTAL_THREADS_IN_PROGRAM;


void
get_shm_handle_name(char *dst, size_t sz)
{
  snprintf(dst, sz, "/mcmini-%s-%lu", getenv("USER"), (long)getpid());
  dst[sz - 1] = '\0';
}

void *
allocate_shared_memory_region()
{
  //  If the region exists, then this returns a fd for the existing
  //  region. Otherwise, it creates a new shared memory region.
  char dpor[100];
  get_shm_handle_name(dpor, sizeof(dpor));

  // This creates a file in /dev/shm/
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
  int rc = ftruncate(fd, shmAllocationSize);
  if (rc == -1) {
    perror("ftruncate");
    mc_exit(EXIT_FAILURE);
  }
  // We want stack at same address for each process.  Otherwise, a
  // pointer
  //   to an address in the stack data structure will not work
  // TODO: Would the following suffice instead?
  //
  // static char large_region[1_000_000];
  // void *shm_map_addr = ;
  //
  // Or do we even need the same location anymore?
  //
  void *stack_address = (void *)0x4444000;
  void *shmStart =
    mmap(stack_address, shmAllocationSize, PROT_READ | PROT_WRITE,
         MAP_SHARED | MAP_FIXED, fd, 0);
  if (shmStart == MAP_FAILED) {
    perror("mmap");
    mc_exit(EXIT_FAILURE);
  }
  // shm_unlink(dpor); // Don't unlink while child processes need to
  // open this.
  fsync(fd);
  close(fd);
  return shmStart;
}

void
deallocate_shared_memory_region()
{
  // TODO: Deallocation
  char shm_file_name[100];
  mc_get_shm_handle_name(shm_file_name, sizeof(shm_file_name));
  int rc = munmap(shmStart, shmAllocationSize);
  if (rc == -1) {
    perror("munmap");
    mc_exit(EXIT_FAILURE);
  }

  rc = shm_unlink(shm_file_name);
  if (rc == -1) {
    if (errno == EACCES) {
      fprintf(stderr,
              "Shared memory region '%s' not owned by this process\n",
              shm_file_name);
    } else {
      perror("shm_unlink");
    }
    mc_exit(EXIT_FAILURE);
  }
}

void
initialize_shared_memory_globals()
{
  // TODO: We can do this just as in mcmini_private.hpp
  void *shm = allocate_shared_memory_region();
  void *threadQueueStart = shm;
  void *transitionTypeInfoStart = (char *)threadQueueStart + sizeof(*trace_list);
  void *transitionDataStart = (char *)transitionTypeInfoStart + sizeof(*shmTransitionTypeInfo);
  shmStart = shm;
  trace_list = (shared_sem_ref)threadQueueStart;
  shmTransitionTypeInfo = (struct SharedTransition *)transitionTypeInfoStart;
  shmTransitionData = transitionDataStart;
}

void intialize_trace_list()
{
  for (int i = 0; i < MAX_TOTAL_THREADS_IN_PROGRAM; i++) {
    shared_sem_destroy(&(*trace_list)[i]);
    shared_sem_init(&(*trace_list)[i]);
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

__attribute__((constructor)) void my_ctor() {
  // Do something here with the constructor (e.g. dlsym preparation)
  //load_intercepted_symbol_addresses(); a function wrappers.c

  /* Open shm file to discover sem_t region + read/write loc for McMini to read from  etc*/
    initialize_shared_memory_globals();
 
  //Discover the sem_t region and read/write loc
  intialize_trace_list();
  
}