#include "mcmini/misc/cond/MCWakeGroup.hpp"
#include <algorithm>

using namespace mcmini;
using namespace std;

bool
WakeGroup::containsThread(tid_t tid) const
{
  return this->threadSet.count(tid) > 0;
}

WakeGroup::WakeGroup(const std::vector<tid_t> &vec)
  : WakeGroup(WakeGroup::Type::signal, vec)
{}

WakeGroup::WakeGroup(WakeGroup::Type type) : type(type) {}

WakeGroup::WakeGroup(WakeGroup::Type type,
                     const std::vector<tid_t> &vec)
{
  this->threads   = vec;
  this->threadSet = set<tid_t>(vec.begin(), vec.end());
  this->type      = type;
}

void
WakeGroup::wakeThread(tid_t tid)
{
  if (type == Type::signal) {
    this->threads.clear();
    this->threadSet.clear();
  } else {
    removeCandidateThread(tid);
  }
}

void
WakeGroup::removeCandidateThread(tid_t tid)
{
  if (!this->containsThread(tid)) return;

  this->threadSet.erase(tid);
  const auto end =
    remove_if(threads.begin(), threads.end(),
              [tid](const tid_t other) { return other == tid; });
  threads.erase(end, threads.end());
}