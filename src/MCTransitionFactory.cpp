#include "mcmini/MCTransitionFactory.h"
#include "mcmini/transitions/threads/MCThreadCreate.h"
#include "mcmini/transitions/threads/MCThreadJoin.h"

std::shared_ptr<MCTransition>
MCTransitionFactory::createInitialTransitionForThread(
  const std::shared_ptr<MCThread> &thread)
{
  return std::make_shared<MCThreadStart>(thread);
}