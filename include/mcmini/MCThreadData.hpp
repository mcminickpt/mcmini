#ifndef MC_MCTHREADDATA_H
#define MC_MCTHREADDATA_H

#include "mcmini/MCClockVector.hpp"
#include "mcmini/misc/MCSortedStack.hpp"
#include <stack>
#include <stdint.h>
#include <vector>

/**
 * @brief A simple C-like struct that McMini associates
 * with each thread created in the target process
 */
struct MCThreadData final {

  uint32_t getExecutionDepth() const;
  void incrementExecutionDepth();
  void decrementExecutionDepthIfNecessary();

  MCClockVector getClockVector() const;
  void setClockVector(const MCClockVector &);

  // FIXME: We can probably remove execution points
  // from MCThreadData, but a deeper investigation
  // is needed
  uint32_t getLatestExecutionPoint() const;

  void pushNewLatestExecutionPoint(const uint32_t);
  void popLatestExecutionPoint();
  void popExecutionPointsGreaterThan(const uint32_t);
  void resetExecutionData();

private:

  /**
   * @brief The number of transitions that
   * the thread described by this data has
   * executed
   */
  uint32_t executionDepth = 0u;

  /**
   * @brief
   *
   */
  MCSortedStack<uint32_t> executionPoints;

  /**
   * @brief
   *
   */
  MCClockVector clockVector = MCClockVector::newEmptyClockVector();
};

#endif // MC_MCTHREADDATA_H