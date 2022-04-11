#ifndef GMAL_GMALSHAREDTRANSITION_H
#define GMAL_GMALSHAREDTRANSITION_H

#include "GMALShared.h"

struct GMALSharedTransition {
    GMALTransitionID type;
    uint8_t data[];
};

#endif //GMAL_GMALSHAREDTRANSITION_H
