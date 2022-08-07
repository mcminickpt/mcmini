#include "mcmini/objects/MCGlobalVariable.h"

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