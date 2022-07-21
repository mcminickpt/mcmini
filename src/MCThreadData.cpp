#include "MCThreadData.hpp"

uint32_t 
MCThreadData::getExecutionDepth() const
{
    return this->executionDepth;
}

void 
MCThreadData::incrementExecutionDepth()
{
    this->executionDepth++;
}

void 
MCThreadData::decrementExecutionDepthIfNecessary()
{
    this->executionDepth--;
}

void 
MCThreadData::resetExecutionDepth()
{
    this->executionDepth = 0;
}

MCClockVector 
MCThreadData::getClockVector() const
{
    return this->clockVector;
}

void 
MCThreadData::setClockVector(const MCClockVector &cv)
{
    this->clockVector = cv;
}

uint32_t
MCThreadData::getLatestExecutionPoint() const
{
    return this->latestExecutionPoint;
}

void 
MCThreadData::setLatestExecutionPoint(const uint32_t lep)
{
    this->latestExecutionPoint = lep;
}