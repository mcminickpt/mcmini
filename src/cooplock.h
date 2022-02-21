#ifndef DPOR_COOPLOCK_H
#define DPOR_COOPLOCK_H

#include "decl.h"
#include <semaphore.h>

STRUCT_DECL(coop_lock);
struct coop_lock {
    sem_t dpor_scheduler_sem;
    sem_t pthread_sem;
};

void coop_lock_init(coop_lock_ref);
void coop_lock_destroy(coop_lock_ref);

void coop_wait_thread(coop_lock_ref);
void coop_wait_scheduler(coop_lock_ref);
void coop_wake_thread(coop_lock_ref);
void coop_wake_scheduler(coop_lock_ref);

#endif //DPOR_COOPLOCK_H
