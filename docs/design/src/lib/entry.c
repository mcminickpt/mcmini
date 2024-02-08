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

void
mc_get_shm_handle_name(char *dst, size_t sz)
{
  snprintf(dst, sz, "/mcmini-%s-%lu", getenv("USER"), (long)getpid());
  dst[sz - 1] = '\0';
}

void *
mc_allocate_shared_memory_region()
{
  return NULL;
  // TODO: Allocation
  // //  If the region exists, then this returns a fd for the existing
  // //  region. Otherwise, it creates a new shared memory region.
  // char dpor[100];
  // mc_get_shm_handle_name(dpor, sizeof(dpor));

  // // This creates a file in /dev/shm/
  // int fd = shm_open(dpor, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  // if (fd == -1) {
  //   if (errno == EACCES) {
  //     fprintf(stderr,
  //             "Shared memory region '%s' not owned by this process\n",
  //             dpor);
  //   } else {
  //     perror("shm_open");
  //   }
  //   mc_exit(EXIT_FAILURE);
  // }
  // int rc = ftruncate(fd, shmAllocationSize);
  // if (rc == -1) {
  //   perror("ftruncate");
  //   mc_exit(EXIT_FAILURE);
  // }
  // // We want stack at same address for each process.  Otherwise, a
  // // pointer
  // //   to an address in the stack data structure will not work
  // // TODO: Would the following suffice instead?
  // //
  // // static char large_region[1_000_000];
  // // void *shm_map_addr = ;
  // //
  // // Or do we even need the same location anymore?
  // //
  // void *stack_address = (void *)0x4444000;
  // void *shmStart =
  //   mmap(stack_address, shmAllocationSize, PROT_READ | PROT_WRITE,
  //        MAP_SHARED | MAP_FIXED, fd, 0);
  // if (shmStart == MAP_FAILED) {
  //   perror("mmap");
  //   mc_exit(EXIT_FAILURE);
  // }
  // // shm_unlink(dpor); // Don't unlink while child processes need to
  // // open this.
  // fsync(fd);
  // close(fd);
  // return shmStart;
}

void
mc_deallocate_shared_memory_region()
{
  // TODO: Deallocation
  // char shm_file_name[100];
  // mc_get_shm_handle_name(shm_file_name, sizeof(shm_file_name));
  // int rc = munmap(shmStart, shmAllocationSize);
  // if (rc == -1) {
  //   perror("munmap");
  //   mc_exit(EXIT_FAILURE);
  // }

  // rc = shm_unlink(shm_file_name);
  // if (rc == -1) {
  //   if (errno == EACCES) {
  //     fprintf(stderr,
  //             "Shared memory region '%s' not owned by this process\n",
  //             shm_file_name);
  //   } else {
  //     perror("shm_unlink");
  //   }
  //   mc_exit(EXIT_FAILURE);
  // }
}

void
mc_initialize_shared_memory_globals()
{
  // TODO: We can do this just as in mcmini_private.hpp
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
  void *buf = malloc(10 * sizeof(char));
  memset(buf, (int)('A'), 10 * sizeof(char));

  /* Open shm file to discover sem_t region + read/write loc for McMini to read from  etc*/
  char shm_file_name[100];
  mc_get_shm_handle_name(shm_file_name, sizeof(shm_file_name));
  int fd = shm_open(shm_file_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    if (errno == EACCES) {
      fprintf(stderr,
              "Shared memory region '%s' not owned by this process\n",
              shm_file_name);
    } else {
      perror("shm_open");
    }
    mc_exit(EXIT_FAILURE);
  }
  //Map the shared memory region into the address space of the process
  void *shmStart = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmStart == MAP_FAILED) {
    perror("mmap");
    mc_exit(EXIT_FAILURE);
  }

  //Discover the sem_t region and read/write loc
  sem_t *sem = (sem_t *)shmStart;
  void *read_loc = (void *)((char *)shmStart + sizeof(sem_t));
  void *write_loc = (void *)((char *)read_loc + sizeof(void *));
  munmap(shmStart, 100);

  ((char *)buf)[9] = 0;
  write(STDERR_FILENO, buf, 10);
  free(buf);
}