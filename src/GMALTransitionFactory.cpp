//
// Created by parallels on 4/10/22.
//

#include "GMALTransitionFactory.h"
std::shared_ptr<GMALTransition>
GMALTransitionFactory::createInitialTransitionForThread(std::shared_ptr<GMALThread> thread)
{
    auto tStart = new GMALThreadStart(thread);
    return std::shared_ptr<GMALTransition>(tStart);
}