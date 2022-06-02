#ifndef MC_MCSTATECONFIGURATION_H
#define MC_MCSTATECONFIGURATION_H

#include "MCShared.h"

/**
 * A configuration constant which specifies that threads
 * may execute as many transitions as they would like (i.e. are
 * not limited to an execution depth)
 */
#define MC_STATE_CONFIG_THREAD_NO_LIMIT             (UINT64_MAX)
#define MC_STATE_CONFIG_NO_TRACE                    (UINT64_MAX)
#define MC_STAT_CONFIG_NO_TRANSITION_STACK_DUMP     (UINT64_MAX)

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
     * Whether or not the model
     * checker stops when it encounters
     * a deadlock
     */
    const bool stopAtFirstDeadlock;

    /**
     * The number of additional transitions mcmini gives to
     * threads in order to detect starvation
     */
    const uint64_t extraLivenessTransitions;

    /**
     * The maxmimum number of transitions that
     * mcmini will run in total before preventing
     * threads from running any further
     */
    const uint64_t maxCutoffDepth;

    MCStateConfiguration(uint64_t maxThreadExecutionDepth,
                           trid_t gdbDebugTraceNumber,
                           trid_t stackContentDumpTraceNumber,
                           bool firstDeadlock,
                          uint64_t extraLivenessTransitions,
                          uint64_t maxCutoffDepth)
    : maxThreadExecutionDepth(maxThreadExecutionDepth),
      gdbDebugTraceNumber(gdbDebugTraceNumber),
      stackContentDumpTraceNumber(stackContentDumpTraceNumber),
      extraLivenessTransitions(extraLivenessTransitions),
      stopAtFirstDeadlock(firstDeadlock),
      maxCutoffDepth(maxCutoffDepth) {}
};

#endif //MC_MCSTATECONFIGURATION_H
