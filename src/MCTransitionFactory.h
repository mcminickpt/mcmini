#ifndef MC_MCTRANSITIONFACTORY_H
#define MC_MCTRANSITIONFACTORY_H

#include "MCShared.h"
#include "MCTransition.h"
#include "transitions/threads/MCThreadStart.h"

namespace mcmini {

class TransitionFactory final {
public:
  TransitionFactory() = delete;

  // Factory methods
  static std::shared_ptr<MCTransition>
  createInitialTransitionForThread(
    const std::shared_ptr<MCThread> &thread);
};

} // namespace mcmini

using MCTransitionFactory = mcmini::TransitionFactory;

#endif // MC_MCTRANSITIONFACTORY_H
