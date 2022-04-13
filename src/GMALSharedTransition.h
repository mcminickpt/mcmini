#ifndef GMAL_GMALSHAREDTRANSITION_H
#define GMAL_GMALSHAREDTRANSITION_H

#include "GMALShared.h"
#include "GMALTransition.h"

struct GMALSharedTransition {
public:
    const std::type_info &type;
    uint8_t data[];
    GMALSharedTransition(const std::type_info &type) : type(type) {}
};

#endif //GMAL_GMALSHAREDTRANSITION_H
