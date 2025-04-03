#include <stdio.h>
#include "mcmini/spy/intercept/interception.h"
#include "mcmini/spy/intercept/wrappers.hpp"
#include "mcmini/real_world/shm.hpp"  // Include the shared memory functionality

extern sem_t read_sem;
extern sem_t write_sem;

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  printf("Hello from mc_pthread_mutext_init!");
  sem_wait(&write_sem);
  mcmini::real_world::shared_memory_region shm_region("/your_shm_name", 100); // Adjust the shm_name and size accordingly

  // Create and write transition information to the shared memory region
  MCMutexShadow mutexShadow(mutex);
  MCMutexInit mutexInitTransition(mutexattr, mutex);

  // Serialize transition information and copy to shared memory
  size_t transition_size = sizeof(MCMutexInit);  // Adjust based on actual size
  size_t offset = 0;  // Adjust based on your data structure

  memcpy(const_cast<void*>(shm_region.get()) + offset, &mutexInitTransition, transition_size);

  // Release the read semaphore to notify the coordinator that new data is avai

  return 0;
}