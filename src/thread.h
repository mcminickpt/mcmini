#include <pthread.h>

struct thread { 
  pthread_t owner;
  int is_alive;
  void *(*start_routine) (void *);
  void *arg;
};
