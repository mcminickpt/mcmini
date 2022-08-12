#include "mcmini/misc/cond/MCWakeGroup.hpp"
#include <algorithm>

using namespace mcmini;
using namespace std;

bool
WakeGroup::containsThread(tid_t tid) const
{
  return this->threadSet.count(tid) > 0;
}

void
WakeGroup::wakeThread(tid_t tid)
{
  // FIXME: Wake threads
}

void
WakeGroup::removeCandidateThread(tid_t tid)
{
  if (!this->containsThread(tid)) return;

  this->threadSet.erase(tid);

  //   remove_if(threadSet.begin(), threadSet.end(), );/
}