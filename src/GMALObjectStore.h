#ifndef GMAL_GMALOBJECTSTORE_H
#define GMAL_GMALOBJECTSTORE_H

#include "GMALVisibleObject.h"
#include "GMALShared.h"
#include <memory>
#include <string.h>

class GMALObjectStore {
private:

    objid_t storageTop;
    std::shared_ptr<GMALVisibleObject> storage[MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM];

    inline objid_t
    _registerNewObject(GMALVisibleObject *object)
    {
        objid_t newObjectId = storageTop++;
        GMAL_ASSERT(newObjectId < MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM);
        storage[newObjectId] = std::shared_ptr<GMALVisibleObject>(object);
        return newObjectId;
    }

public:

    inline GMALObjectStore() : storageTop(0) { bzero(storage, sizeof(storage)); }

    inline objid_t
    registerNewObject(GMALVisibleObject *object)
    {
        return this->_registerNewObject(object);
    }

    inline objid_t
    softRegisterNewObject(GMALVisibleObject *object)
    {
        return this->_registerNewObject(object);
    }

    template<typename Object>
    inline std::shared_ptr<Object>
    getObjectWithId(objid_t id) const
    {
        return std::static_pointer_cast<Object, GMALVisibleObject>(this->storage[id]);
    }
};

#endif //GMAL_GMALOBJECTSTORE_H
