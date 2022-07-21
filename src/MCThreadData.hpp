#ifndef MC_MCTHREADDATA_H
#define MC_MCTHREADDATA_H

#include <stdint.h>
#include <vector>
#include "MCClockVector.hpp"

/**
 * @brief A simple C-like struct that McMini associates
 * with each thread created in the target process
 */
struct MCThreadData final {

    uint32_t getExecutionDepth() const;
    void incrementExecutionDepth();
    void decrementExecutionDepthIfNecessary();
    void resetExecutionDepth();

    MCClockVector getClockVector() const;
    void setClockVector(const MCClockVector &);

    uint32_t getLatestExecutionPoint() const;
    void setLatestExecutionPoint(const uint32_t);

private:

    uint32_t executionDepth = 0u;

    uint32_t latestExecutionPoint = 0u;

    MCClockVector clockVector = MCClockVector::newEmptyClockVector();
};

#endif //MC_MCTHREADDATA_H