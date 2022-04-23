#ifndef GMAL_GMALSEMAPHORETRANSITIONWRAPPERS_H
#define GMAL_GMALSEMAPHORETRANSITIONWRAPPERS_H

#include "GMALShared.h"
#include <semaphore.h>

int gmal_sem_init(sem_t*, int, unsigned int);
int gmal_sem_post(sem_t*);
int gmal_sem_wait(sem_t*);

#endif //GMAL_GMALSEMAPHORETRANSITIONWRAPPERS_H
