#ifndef GMAL_GMALSTATECONFIGURATION_H
#define GMAL_GMALSTATECONFIGURATION_H

#include "GMALShared.h"

#define GMAL_STATE_CONFIG_THREAD_NO_LIMIT UINT32_MAX


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

    GMALStateConfiguration(uint32_t maxThreadExecutionDepth) : maxThreadExecutionDepth(maxThreadExecutionDepth) {}
};

#endif //GMAL_GMALSTATECONFIGURATION_H
