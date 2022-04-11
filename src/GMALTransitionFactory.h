#ifndef GMAL_GMALTRANSITIONFACTORY_H
#define GMAL_GMALTRANSITIONFACTORY_H

#include "GMALShared.h"
#include "GMALRef.h"
#include "GMALTransition.h"
#include "GMALThreadStart.h"

class GMALTransitionFactory final {
public:
    GMALTransitionFactory() = delete;

    static std::shared_ptr<GMALTransition>
    createInitialTransitionForThread(GMALThread *thread)
    {
        auto tStart = new GMALThreadStart(GMAL_PASS_DYNAMIC<GMALThread>(thread));
        return std::shared_ptr<GMALTransition>(tStart);
    }
};

#endif //GMAL_GMALTRANSITIONFACTORY_H
