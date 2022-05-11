#ifndef GMAL_GMALVISIBLEOBJECT_H
#define GMAL_GMALVISIBLEOBJECT_H

class GMALState;
class GMALObjectStore;

#include "GMALShared.h"
#include <memory>

class GMALVisibleObject {
    objid_t id;
    friend GMALObjectStore;
protected:
    GMALVisibleObject() = default;
    GMALVisibleObject(objid_t id) : id(id) {}
public:
    virtual std::shared_ptr<GMALVisibleObject> copy() = 0;

    virtual GMALSystemID getSystemId() = 0;
    objid_t getObjectId() const;
};

#endif //GMAL_GMALVISIBLEOBJECT_H
