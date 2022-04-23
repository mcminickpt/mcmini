#ifndef GMAL_GMALSEMAPHORETRANSITION_H
#define GMAL_GMALSEMAPHORETRANSITION_H

#include "objects/GMALSemaphore.h"
#include "GMALTransition.h"

struct GMALSemaphoreTransition : public GMALTransition {
public:
    std::shared_ptr<GMALSemaphore> sem;
    GMALSemaphoreTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALSemaphore> sem) :
            GMALTransition(running), sem(sem) {}
};

#endif //GMAL_GMALSEMAPHORETRANSITION_H
