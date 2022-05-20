#ifndef MC_MCSTATECONFIGURATION_H
#define MC_MCSTATECONFIGURATION_H

#include "MCShared.h"

/**
 * A configuration constant which specifies that threads
 * may execute as many transitions as they would like (i.e. are
 * not limited to an execution depth)
 */
#define MC_STATE_CONFIG_THREAD_NO_LIMIT UINT64_MAX
#define MC_STATE_CONFIG_NO_TRACE UINT64_MAX

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

    MCStateConfiguration(uint64_t maxThreadExecutionDepth, trid_t gdbDebugTraceNumber)
    : maxThreadExecutionDepth(maxThreadExecutionDepth), gdbDebugTraceNumber(gdbDebugTraceNumber) {}
};

#endif //MC_MCSTATECONFIGURATION_H
