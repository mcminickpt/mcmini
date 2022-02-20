#ifndef DPOR_OTHREAD_H
#define DPOR_OTHREAD_H

#include <semaphore.h>
#include "decl.h"
#include "thread.h"
#include "array.h"

STRUCT_DECL(othread);
struct othread {
    thread_ref thread;          /* The thread which waits*/
    sem_t dpor_scheduler_sem;   /* The semaphore the DPOR scheduler waits on */
    sem_t pthread_sem;          /* The semaphore the user thread waits on */
};
typedef array_ref othread_array_ref;

othread_ref othread_alloc(void);
othread_ref othread_create(thread_ref);
othread_ref othread_copy(othread_refc);
void othread_destroy(othread_ref);

othread_ref othread_pthread_wrap(pthread_t);

#endif //DPOR_OTHREAD_H
