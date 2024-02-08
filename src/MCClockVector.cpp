#include "mcmini/MCClockVector.hpp"

using namespace std;

MCClockVector
MCClockVector::max(const MCClockVector &cv1, const MCClockVector &cv2)
{
  const uint32_t c1Size                     = cv1.size();
  const uint32_t c2Size                     = cv2.size();
  const unordered_map<tid_t, uint32_t> &um1 = cv1.contents;
  const unordered_map<tid_t, uint32_t> &um2 = cv2.contents;
  const auto ume1                           = um1.end();
  const auto ume2                           = um2.end();
  MCClockVector maxCV = MCClockVector::newEmptyClockVector();

  for (const auto &c1Elem : um1) {
    const tid_t &tid       = c1Elem.first;
    const uint32_t &index  = c1Elem.second;
    const uint32_t c2Index = cv2.valueForThread(tid).value_or(0);
    maxCV[tid]             = std::max(index, c2Index);
  }

  for (const auto &c2Elem : um2) {
    const tid_t &tid       = c2Elem.first;
    const uint32_t &index  = c2Elem.second;
    const uint32_t c1Index = cv1.valueForThread(tid).value_or(0);
    maxCV[tid]             = std::max(index, c1Index);
  }

  return maxCV;
}