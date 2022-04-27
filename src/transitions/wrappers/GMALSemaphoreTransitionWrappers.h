#ifndef GMAL_GMALSEMAPHORETRANSITIONWRAPPERS_H
#define GMAL_GMALSEMAPHORETRANSITIONWRAPPERS_H

#include <semaphore.h>
#include "GMALShared.h"

GMAL_EXTERN int gmal_sem_init(sem_t*, int, unsigned int);
GMAL_EXTERN int gmal_sem_post(sem_t*);
GMAL_EXTERN int gmal_sem_wait(sem_t*);

#endif //GMAL_GMALSEMAPHORETRANSITIONWRAPPERS_H
