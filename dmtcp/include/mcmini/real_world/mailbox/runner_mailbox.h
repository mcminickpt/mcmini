#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <semaphore.h>
#include <stdint.h>

typedef struct {
  sem_t model_side_sem;
  sem_t child_side_sem;
  uint32_t type;
  uint8_t cnts[64];  // TODO: How much space should each thread have to write
                     // payloads?
} runner_mailbox, *runner_mailbox_ref;

void mc_runner_mailbox_init(volatile runner_mailbox *);
void mc_runner_mailbox_destroy(volatile runner_mailbox *);
int mc_wait_for_thread(volatile runner_mailbox *);
int mc_wait_for_scheduler(volatile runner_mailbox *);
int mc_wake_thread(volatile runner_mailbox *);
int mc_wake_scheduler(volatile runner_mailbox *);

#ifdef __cplusplus
}
#endif  // extern "C"
