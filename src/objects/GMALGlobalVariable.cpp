#include "GMALGlobalVariable.h"

bool
GMALGlobalVariable::operator==(const GMALGlobalVariable &other) const
{
    return this->addr == other.addr;
}

bool
GMALGlobalVariable::operator!=(const GMALGlobalVariable &other) const
{
    return this->addr != other.addr;
}

GMALSystemID
GMALGlobalVariable::getSystemId()
{
    return (GMALSystemID)addr;
}

std::shared_ptr<GMALVisibleObject>
GMALGlobalVariable::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALGlobalVariable(*this));
}