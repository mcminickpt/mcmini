#include <stdio.h>
#include "mcmini/spy/intercept/interception.h"

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  printf("Hello from mc_pthread_mutext_init!");
  return 0;
}