#ifndef GMAL_GMALSTATECONFIGURATION_H
#define GMAL_GMALSTATECONFIGURATION_H

#include "GMALShared.h"

/**
 * A configuration constant which specifies that threads
 * may execute as many transitions as they would like (i.e. are
 * not limited to an execution depth)
 */
#define GMAL_STATE_CONFIG_THREAD_NO_LIMIT UINT32_MAX
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
    const uint32_t maxThreadExecutionDepth;

    /**
     * The trace id to stop the model checker at
     * to allow GDB to run through a trace
     */
    const trid_t gdbDebugTraceNumber;

    GMALStateConfiguration(uint32_t maxThreadExecutionDepth, trid_t gdbDebugTraceNumber)
    : maxThreadExecutionDepth(maxThreadExecutionDepth), gdbDebugTraceNumber(gdbDebugTraceNumber) {}
};

#endif //GMAL_GMALSTATECONFIGURATION_H
