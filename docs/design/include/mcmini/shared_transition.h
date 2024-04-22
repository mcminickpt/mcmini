#ifndef INCLUDE_SHARED_TRANSITION_H
#define INCLUDE_SHARED_TRANSITION_H
#include <stdint.h>

typedef uint64_t tid_t;

typedef enum {
  mutex_init,
  mutex_lock,
  mutex_unlock,
  sem_init,
  sem_wait,
  sem_post,
  thread_create,
  thread_join
  //...
} transition_type;

struct shared_transition {
  tid_t executor;
  transition_type type;
};

#endif