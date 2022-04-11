#ifndef GMAL_GMAL_H
#define GMAL_GMAL_H

#include "GMALShared.h"
#include "GMALSharedTransition.h"

extern "C" {
    #include <semaphore.h>
    #include "mc_shared_cv.h"
}

void gmal_init();

#endif //GMAL_GMAL_H
