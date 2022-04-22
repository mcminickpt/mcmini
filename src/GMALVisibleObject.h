#ifndef GMAL_GMALVISIBLEOBJECT_H
#define GMAL_GMALVISIBLEOBJECT_H

class GMALState;

#include "GMALShared.h"
#include "GMALVisibleObject.h"
#include <memory>

class GMALVisibleObject {
    objid_t id;
    virtual GMALSystemID getSystemId() = 0;
    virtual std::shared_ptr<GMALVisibleObject> copy() = 0;
    friend GMALState;
protected:
    GMALVisibleObject() = default;
    GMALVisibleObject(objid_t id) : id(id) {}
public:
    objid_t getObjectId() const;
};

#endif //GMAL_GMALVISIBLEOBJECT_H
