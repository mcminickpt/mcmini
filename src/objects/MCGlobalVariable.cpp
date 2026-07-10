#include "objects/MCGlobalVariable.h"

bool
MCGlobalVariable::operator==(const MCGlobalVariable &other) const
{
  return this->addr == other.addr;
}

bool
MCGlobalVariable::operator!=(const MCGlobalVariable &other) const
{
  return this->addr != other.addr;
}

bool
MCGlobalVariable::MCObjectEquals(const MCVisibleObject &other) const
{
  const MCGlobalVariable *otherVar =
    dynamic_cast<const MCGlobalVariable *>(&other);
  if (!otherVar) {
    return false;
  }
  return this->addr == otherVar->addr && this->val == otherVar->val;
}

MCSystemID
MCGlobalVariable::getSystemId()
{
  return (MCSystemID)addr;
}

std::shared_ptr<MCVisibleObject>
MCGlobalVariable::copy()
{
  return std::shared_ptr<MCVisibleObject>(
    new MCGlobalVariable(*this));
}
