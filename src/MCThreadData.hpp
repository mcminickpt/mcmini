#ifndef MC_MCTHREADDATA_H
#define MC_MCTHREADDATA_H

#include <stdint.h>
#include <vector>
#include "MCClockVector.hpp"

struct MCThreadData final {
    void incrementExecutionDepth();
    uint32_t getExecutionDepth();
    void decrementExecutionDepthIfNecessary();

    MCClockVector getClockVector();
    void setClockVector(const MCClockVector &);
};

#endif //MC_MCTHREADDATA_H