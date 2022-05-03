#include "GMALConditionVariable.h"

std::shared_ptr<GMALVisibleObject>
GMALConditionVariable::copy()
{
    return std::shared_ptr<GMALVisibleObject>(new GMALConditionVariable(*this));
}

GMALSystemID
GMALConditionVariable::getSystemId()
{
    return this->condShadow.cond;
}

bool
GMALConditionVariable::operator==(const GMALConditionVariable &other) const
{
    return this->condShadow.cond == other.condShadow.cond;
}

bool
GMALConditionVariable::operator!=(const GMALConditionVariable &other) const
{
    return this->condShadow.cond != other.condShadow.cond;
}

bool
GMALConditionVariable::isOwned() const
{
    // TODO: Implement this
    return false;
}

void
GMALConditionVariable::relinquish()
{
    this->mutex->unlock();
}

void
GMALConditionVariable::takeOwnership()
{
    this->mutex->lock();
}

void
GMALConditionVariable::print()
{

}
