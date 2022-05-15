#ifndef GMAL_GMALSTATECONFIGURATION_H
#define GMAL_GMALSTATECONFIGURATION_H

#include "GMALShared.h"

/**
 * A configuration constant which specifies that threads
 * may execute as many transitions as they would like (i.e. are
 * not limited to an execution depth)
 */
#define GMAL_STATE_CONFIG_THREAD_NO_LIMIT UINT64_MAX
#define GMAL_STATE_CONFIG_NO_TRACE UINT64_MAX

/**
 * A struct which describes the configurable parameters
 * of the model checking execution
 */
struct GMALStateConfiguration final {

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
     * Whether or not this model checking session is
     * being used to check for starvation to make statements
     * about forward progress
     *
     * In order that the model checker can check for forward progress,
     * the target program must be modified with calls to GOAL(), which
     * act as marker for where we wish to ensure threads reach after
     * a fixed number of steps. When this option is set, it should be coupled
     * with a reasonable value for `maxThreadExecutionDepth`
     */
    const bool expectForwardProgressOfThreads;

    GMALStateConfiguration(uint64_t maxThreadExecutionDepth,
                           trid_t gdbDebugTraceNumber,
                           bool expectForwardProgressOfThreads)
    : maxThreadExecutionDepth(maxThreadExecutionDepth),
    gdbDebugTraceNumber(gdbDebugTraceNumber),
    expectForwardProgressOfThreads(expectForwardProgressOfThreads) {}
};

#endif //GMAL_GMALSTATECONFIGURATION_H
