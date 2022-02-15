#ifndef DPOR_OTHREAD_H
#define DPOR_OTHREAD_H

#include <semaphore.h>
#include "decl.h"
#include "thread.h"
#include "array.h"

STRUCT_DECL(othread);
struct othread {
    thread_ref thread;
    sem_t dpor_scheduler_sem; // The semaphore the dpor scheduler waits on
    sem_t pthread_sem; // The semaphore the user thread waits on
};
typedef array_ref othread_array_ref;
MEMORY_API_DECL(othread);

othread_ref othread_pthread_wrap(pthread_t);
othread_ref othread_thread_wrap(thread_ref);


#endif //DPOR_OTHREAD_H
