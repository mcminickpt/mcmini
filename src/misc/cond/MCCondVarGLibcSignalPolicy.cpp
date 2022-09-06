#include "mcmini/misc/cond/MCCondVarGLibcWakeupPolicy.hpp"
#include <algorithm>

using namespace mcmini;
using namespace std;

std::unique_ptr<ConditionVariableSignalPolicy>
CondVarGLibcWakeupPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableSignalPolicy>(
    new CondVarGLibcWakeupPolicy(*this));
}

void
CondVarGLibcWakeupPolicy::addWaiter(tid_t tid)
{
  this->group2.push_back(tid);
}

void
CondVarGLibcWakeupPolicy::removeWaiter(tid_t tid)
{
  vector<tid_t>::iterator iter =
    find(this->group1.begin(), this->group1.end(), tid);
  if (iter != this->group1.end()) this->group1.erase(iter);

  iter = find(this->group2.begin(), this->group2.end(), tid);
  if (iter != this->group2.end()) this->group2.erase(iter);
}

WakeGroup
CondVarGLibcWakeupPolicy::receiveSignalMessage()
{
  this->exchangeGroupsIfNecessary();
  return WakeGroup(WakeGroup::Type::signal, this->group1);
}

WakeGroup
CondVarGLibcWakeupPolicy::receiveBroadcastMessage()
{
  this->exchangeGroupsIfNecessary();
  return WakeGroup(WakeGroup::Type::broadcast, this->group1);
}

void
CondVarGLibcWakeupPolicy::exchangeGroupsIfNecessary()
{
  if (this->group1.empty()) {
    this->group1 = this->group2;
    this->group2.clear();
  }
}