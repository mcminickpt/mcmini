#include "mcmini/misc/cond/MCWakeGroup.hpp"
#include <algorithm>

namespace mcmini {

bool
WakeGroup::contains(tid_t tid) const
{
  return std::find(begin(), end(), tid) != end();
}

void
WakeGroup::remove_candidate_thread(tid_t tid)
{
  erase(std::remove(begin(), end(), tid), end());
}

} // namespace mcmini