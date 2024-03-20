#include "mcmini/shared_sem.h"
#include "mcmini/wrappers.h"

void shared_sem_init(shared_sem_ref ref) {
  if (!ref) return;
  __real_sem_init(&ref->runner_sem, 1, 0);
    __real_sem_init(&ref->pthread_sem, 1, 0);
}

void shared_sem_destroy(shared_sem_ref ref) {
  if (!ref) return;
  __real_sem_destroy(&ref->runner_sem);
  __real_sem_destroy(&ref->pthread_sem);
}

void shared_sem_post_for_thread(shared_sem_ref ref) {
  if (!ref) return;
  __real_sem_post(&ref->pthread_sem);
}

void shared_sem_wait_for_thread(shared_sem_ref ref) {
  if (!ref) return;
  __real_sem_wait(&ref->pthread_sem);
}

void shared_sem_post_for_runner(shared_sem_ref ref) {
  if (!ref) return;
  __real_sem_post(&ref->runner_sem);
}

void shared_sem_wait_for_runner(shared_sem_ref ref) {
  if (!ref) return;
  __real_sem_wait(&ref->runner_sem);
}

