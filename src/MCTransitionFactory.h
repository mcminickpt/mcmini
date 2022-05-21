#ifndef MC_MCTRANSITIONFACTORY_H
#define MC_MCTRANSITIONFACTORY_H

#include "MCShared.h"
#include "MCTransition.h"
#include "transitions/threads/MCThreadStart.h"

class MCTransitionFactory final {
public:
    MCTransitionFactory() = delete;

    // Factory methods
    static std::shared_ptr<MCTransition> createInitialTransitionForThread(std::shared_ptr<MCThread> thread);
    static bool transitionsCoenabledCommon(const std::shared_ptr<MCTransition> &t1, const std::shared_ptr<MCTransition> &t2);
    static bool transitionsDependentCommon(const std::shared_ptr<MCTransition> &t1, const std::shared_ptr<MCTransition> &t2);
};

#endif //MC_MCTRANSITIONFACTORY_H
