#ifndef GMAL_GMALTRANSITIONFACTORY_H
#define GMAL_GMALTRANSITIONFACTORY_H

#include "GMALShared.h"
#include "GMALTransition.h"
#include "transitions/GMALThreadStart.h"

class GMALTransitionFactory final {
public:
    GMALTransitionFactory() = delete;

    // Factory methods
    static std::shared_ptr<GMALTransition> createInitialTransitionForThread(std::shared_ptr<GMALThread> thread);


    static bool transitionsCoenabledCommon(const std::shared_ptr<GMALTransition> &t1, const std::shared_ptr<GMALTransition> &t2);
    static bool transitionsDependentCommon(const std::shared_ptr<GMALTransition> &t1, const std::shared_ptr<GMALTransition> &t2);
};

#endif //GMAL_GMALTRANSITIONFACTORY_H
