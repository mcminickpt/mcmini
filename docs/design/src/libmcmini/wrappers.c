#include <stdio.h>
#include "mcmini/spy/intercept/interception.h"

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  printf("Hello from mc_pthread_mutext_init!");
  // TODO: write into the shm region enough information
  // to determine what just happened on the model side

  // The coordinator first assumes data is written as follows:
  // transition id followed by payload.
  //
  // TODO: There's no system in place to synchronize transition ids
  // with the registration on the model side. This is a dynamic process
  // (new transition can be added at runtime)
  // For now, it suffices to assign a fixed value and just assume it
  // corresponds on the model side
  return 0;
}