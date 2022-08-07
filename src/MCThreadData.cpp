#include "mcmini/MCThreadData.hpp"

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
MCThreadData::resetExecutionData()
{
  this->executionDepth  = 0;
  this->executionPoints = MCSortedStack<uint32_t>();
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
  if (this->executionPoints.empty()) return static_cast<uint32_t>(0);
  return this->executionPoints.top();
}

void
MCThreadData::pushNewLatestExecutionPoint(const uint32_t depth)
{
  if (!this->executionPoints.empty()) {
    MC_ASSERT(this->executionPoints.top() <= depth);
  }
  this->executionPoints.push(depth);
}

void
MCThreadData::popLatestExecutionPoint()
{
  this->executionPoints.pop();
}

void
MCThreadData::popExecutionPointsGreaterThan(const uint32_t index)
{
  this->executionPoints.popGreaterThan(index);
}