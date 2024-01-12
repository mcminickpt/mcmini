#include <iostream>

#include "mcmini/spy/intercept/interception.hpp"

int mc_pthread_mutex_init(pthread_mutex_t *mutex,
                          const pthread_mutexattr_t *mutexattr) {
  std::cout << "Hello from mutex init!" << std::endl;
  return 0;
}