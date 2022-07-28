#include "MCTransitionFactory.h"
#include "transitions/threads/MCThreadCreate.h"
#include "transitions/threads/MCThreadJoin.h"

std::shared_ptr<MCTransition>
MCTransitionFactory::createInitialTransitionForThread(const std::shared_ptr<MCThread> &thread)
{
    return std::make_shared<MCThreadStart>(thread);
}