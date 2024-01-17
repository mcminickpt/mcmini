#include "mcmini/spy/intercept/interception.hpp"

#include <cstdio>
#include <cstdlib>

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  std::printf("Hello from %s", __FUNCTION__);
  return 0;
}