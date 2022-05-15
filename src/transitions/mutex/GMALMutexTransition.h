#ifndef GMAL_GMALMUTEXTRANSITION_H
#define GMAL_GMALMUTEXTRANSITION_H

#include "GMALTransition.h"
#include "objects/GMALMutex.h"

struct GMALMutexTransition : public GMALTransition {
public:
    std::shared_ptr<GMALMutex> mutex;
    GMALMutexTransition(std::shared_ptr<GMALThread> running, std::shared_ptr<GMALMutex> mutex) :
    GMALTransition(running), mutex(mutex) {}
};

#endif //GMAL_GMALMUTEXTRANSITION_H
