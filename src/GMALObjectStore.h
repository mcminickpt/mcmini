#ifndef GMAL_GMALOBJECTSTORE_H
#define GMAL_GMALOBJECTSTORE_H

#include "objects/GMALVisibleObject.h"
#include "GMALShared.h"
#include <memory>
#include <unordered_map>
#include <string.h>

struct PointerHasher {
    std::size_t operator()(void *code) const
    {
        return (std::size_t)(code);
    }
};

struct PointersEqual {
    bool operator()(void *lhs, void *rhs) const
    {
        return lhs == rhs;
    }
};

class GMALObjectStore {
private:

    struct StorageObject final {
        std::shared_ptr<GMALVisibleObject> current;
        const std::shared_ptr<GMALVisibleObject> initialState;

        StorageObject(std::shared_ptr<GMALVisibleObject> current, std::shared_ptr<GMALVisibleObject> initialState)
        : current(current), initialState(initialState) {}
    };

    objid_t storageTop = -1; /* Points to the most recent item in the storage */
    std::shared_ptr<StorageObject> storage[MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM];

    /**
     * Maps identities of visible objects given by the system to their
     * shadow-struct counterparts in
     */
    std::unordered_map<GMALSystemID, objid_t, PointerHasher, PointersEqual> systemVisibleObjectMap;

    inline objid_t
    _registerNewObject(std::shared_ptr<GMALVisibleObject> object)
    {
        objid_t newObjectId = ++storageTop;
        GMAL_ASSERT(newObjectId < MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM);
        object->id = newObjectId;
        const auto initialState = object->copy();
        storage[newObjectId] = std::make_shared<StorageObject>(object, initialState);
        return newObjectId;
    }

public:

    inline GMALObjectStore() { bzero(storage, sizeof(storage)); }

    inline objid_t
    registerNewObject(std::shared_ptr<GMALVisibleObject> object)
    {
        return this->_registerNewObject(object);
    }

    template<typename Object>
    inline std::shared_ptr<Object>
    getObjectWithId(objid_t id) const
    {
        return std::static_pointer_cast<Object, GMALVisibleObject>(this->storage[id]->current);
    }

    inline void
    mapSystemAddressToShadow(GMALSystemID systemAddress, objid_t shadowId)
    {
        systemVisibleObjectMap.insert({systemAddress, shadowId});
    }

    template<typename Object>
    inline std::shared_ptr<Object>
    getObjectWithSystemAddress(void *systemAddress)
    {
        auto kvPair = systemVisibleObjectMap.find(systemAddress);

        if (kvPair != systemVisibleObjectMap.end()) {
            objid_t shadowObjectId = kvPair->second;
            return this->getObjectWithId<Object>(shadowObjectId);
        } else {
            return nullptr;
        }
    }

    void resetObjectsToInitialStateInStore();
};

#endif //GMAL_GMALOBJECTSTORE_H
