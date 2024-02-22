#ifndef INCLUDE SHARE_SEM_H
#define INCLUDE_SHARE_SEM_H

#include <semaphore.h>
#define MC_STRUCT_DECL(type)       \
  typedef struct type type;        \
  typedef struct type *type##_t;   \
  typedef struct type *type##_ref; \
  typedef const struct type *type##_refc;

MC_STRUCT_DECL(shared_sem)
struct shared_sem {
  sem_t runner_sem; //previously named dpor_scheduler_sem
  sem_t pthread_sem;
};

void shared_sem_init(shared_sem_ref);
void shared_sem_destroy(shared_sem_ref);

void shared_sem_post_for_thread(shared_sem_ref);
void shared_sem_wait_for_thread(shared_sem_ref);

void shared_sem_post_for_runner(shared_sem_ref);
void shared_sem_wait_for_runner(shared_sem_ref);

void shared_sem_wake_runner(shared_sem_ref);
void shared_sem_wake_thread(shared_sem_ref);

#endif