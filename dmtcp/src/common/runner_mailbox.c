#include "mcmini/real_world/mailbox/runner_mailbox.h"

#include "mcmini/defines.h"
#include "string.h"

void mc_runner_mailbox_init(volatile runner_mailbox* r) {
  runner_mailbox_ref ref = (runner_mailbox_ref)(r);
  sem_init(&ref->model_side_sem, SEM_FLAG_SHARED, 0);
  sem_init(&ref->child_side_sem, SEM_FLAG_SHARED, 0);
  memset(ref->cnts, 0, sizeof(ref->cnts));
}

void mc_runner_mailbox_destroy(volatile runner_mailbox* r) {
  runner_mailbox_ref ref = (runner_mailbox_ref)(r);
  sem_destroy(&ref->model_side_sem);
  sem_destroy(&ref->child_side_sem);
}

int mc_wait_for_thread(volatile runner_mailbox* r) {
  runner_mailbox_ref ref = (runner_mailbox_ref)(r);
  return sem_wait(&ref->model_side_sem);
}

int mc_wait_for_scheduler(volatile runner_mailbox* r) {
  runner_mailbox_ref ref = (runner_mailbox_ref)(r);
  return sem_wait(&ref->child_side_sem);
}

int mc_wake_thread(volatile runner_mailbox* r) {
  runner_mailbox_ref ref = (runner_mailbox_ref)(r);
  return sem_post(&ref->child_side_sem);
}

int mc_wake_scheduler(volatile runner_mailbox* r) {
  runner_mailbox_ref ref = (runner_mailbox_ref)(r);
  return sem_post(&ref->model_side_sem);
}
