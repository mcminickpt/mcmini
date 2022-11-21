#ifndef MC_MCSTATECONFIGURATION_H
#define MC_MCSTATECONFIGURATION_H

#include "mcmini/MCShared.h"

/**
 * A configuration constant which specifies that threads
 * may execute as many transitions as they would like (i.e. are
 * not limited to an execution depth)
 */
#define MC_STATE_CONFIG_THREAD_NO_LIMIT         (UINT64_MAX)
#define MC_STATE_CONFIG_NO_GDB_TRACE            (UINT64_MAX)
#define MC_STAT_CONFIG_NO_TRANSITION_STACK_DUMP (UINT64_MAX)

/**
 * A struct which describes the configurable parameters
 * of the model checking execution
 */
struct MCStateConfiguration final {

  /**
   * The maximum number of transitions that can be run
   * by any single thread while running the model checker
   */
  const uint64_t maxThreadExecutionDepth;

  /**
   * The trace id to stop the model checker at
   * to allow GDB to run through a trace
   */
  const trid_t gdbDebugTraceNumber;

  /**
   * The trace id to stop the model checker at
   * to print the contents of the transition stack
   */
  const trid_t stackContentDumpTraceNumber;

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

  /**
   * Whether or not the model
   * checker stops when it encounters
   * a deadlock
   */
  const bool stopAtFirstDeadlock;

  MCStateConfiguration(uint64_t maxThreadExecutionDepth,
                       trid_t gdbDebugTraceNumber,
                       trid_t stackContentDumpTraceNumber,
                       bool firstDeadlock,
                       bool expectForwardProgressOfThreads)
    : maxThreadExecutionDepth(maxThreadExecutionDepth),
      gdbDebugTraceNumber(gdbDebugTraceNumber),
      stackContentDumpTraceNumber(stackContentDumpTraceNumber),
      stopAtFirstDeadlock(firstDeadlock),
      expectForwardProgressOfThreads(expectForwardProgressOfThreads)
  {}
};

#endif // MC_MCSTATECONFIGURATION_H
