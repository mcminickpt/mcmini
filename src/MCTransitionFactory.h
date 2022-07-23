#ifndef MC_MCTRANSITIONFACTORY_H
#define MC_MCTRANSITIONFACTORY_H

#include "MCShared.h"
#include "MCTransition.h"
#include "transitions/threads/MCThreadStart.h"

class MCTransitionFactory final {
public:
    MCTransitionFactory() = delete;

    // Factory methods
    static std::shared_ptr<MCTransition> createInitialTransitionForThread(const std::shared_ptr<MCThread> &thread);
    static bool transitionsCoenabledCommon(const MCTransition *t1, const MCTransition *t2);
    static bool transitionsDependentCommon(const MCTransition *t1, const MCTransition *t2);
};

#endif //MC_MCTRANSITIONFACTORY_H
