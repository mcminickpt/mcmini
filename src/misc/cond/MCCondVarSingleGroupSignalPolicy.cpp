#include "mcmini/misc/cond/MCCondVarSingleGroupSignalPolicy.hpp"
#include <algorithm>

using namespace std;
using namespace mcmini;

unique_ptr<ConditionVariableSignalPolicy>
CondVarSingleGroupSignalPolicy::clone() const
{
  return std::unique_ptr<CondVarSingleGroupSignalPolicy>(
    new CondVarSingleGroupSignalPolicy(*this));
}

void
CondVarSingleGroupSignalPolicy::addWaiter(tid_t tid)
{
  this->waiters.push_back(tid);
}

void
CondVarSingleGroupSignalPolicy::removeWaiter(tid_t tid)
{
  const vector<tid_t>::iterator iter =
    find(this->waiters.begin(), this->waiters.end(), tid);
  if (iter != this->waiters.end()) this->waiters.erase(iter);
}

WakeGroup
CondVarSingleGroupSignalPolicy::receiveSignalMessage()
{
  return WakeGroup(WakeGroup::Type::signal, this->waiters);
}

WakeGroup
CondVarSingleGroupSignalPolicy::receiveBroadcastMessage()
{
  return WakeGroup(WakeGroup::Type::broadcast, this->waiters);
}