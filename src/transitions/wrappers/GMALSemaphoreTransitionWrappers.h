#ifndef MC_MCSEMAPHORETRANSITIONWRAPPERS_H
#define MC_MCSEMAPHORETRANSITIONWRAPPERS_H

#include <semaphore.h>
#include "MCShared.h"

MC_EXTERN int mc_sem_init(sem_t*, int, unsigned int);
MC_EXTERN int mc_sem_post(sem_t*);
MC_EXTERN int mc_sem_wait(sem_t*);

#endif //MC_MCSEMAPHORETRANSITIONWRAPPERS_H
