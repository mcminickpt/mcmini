#ifndef MC_MCSTATECONFIGURATION_H
#define MC_MCSTATECONFIGURATION_H

#include "MCShared.h"


/**
 * A struct which describes the configurable parameters
 * of the model checking execution
 */
struct MCStackConfiguration final {

  /**
   * The maximum number of transitions that can be run
   * by any single thread while running the model checker
   */
  const uint64_t maxThreadExecutionDepth;

  /**
   * The maximum number of transitions that can be run by
   * all the threads in total while running the model checker
   */
  uint64_t maxTotalTransitionsDepthLimit;

  /**
   * The trace id to stop the model checker at
   * to print the contents of the transition stack.
   */
  const trid_t printBacktraceAtTraceNumber;

  /**
   * Whether or not this model checking session is
   * being used to check for starvation to make statements
   * about forward progress
   *
   * In order that the model checker can check for forward progress,
   * the target program must be modified with calls to GOAL(), which
   * act as marker for where we wish to ensure threads reach after
   * a fixed number of steps. When this option is set, it should be
   * coupled with a reasonable value for `maxThreadExecutionDepth`
   */
  const bool expectForwardProgressOfThreads;

  MCStackConfiguration(uint64_t maxThreadExecutionDepth,
                       uint64_t maxTotalTransitionsDepthLimit,
                       trid_t printBacktraceAtTraceNumber,
                       bool firstDeadlock,
                       bool expectForwardProgressOfThreads)
    : maxThreadExecutionDepth(maxThreadExecutionDepth),
      maxTotalTransitionsDepthLimit(maxTotalTransitionsDepthLimit),
      printBacktraceAtTraceNumber(printBacktraceAtTraceNumber),
      expectForwardProgressOfThreads(expectForwardProgressOfThreads)
  {}
};

#endif // MC_MCSTATECONFIGURATION_H
