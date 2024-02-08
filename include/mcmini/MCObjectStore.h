#ifndef MC_MCOBJECTSTORE_H
#define MC_MCOBJECTSTORE_H

#include "mcmini/MCShared.h"
#include "mcmini/objects/MCVisibleObject.h"
#include <memory>
#include <string.h>
#include <unordered_map>

struct PointerHasher {
  std::size_t
  operator()(void *code) const
  {
    return (std::size_t)(code);
  }
};

struct PointersEqual {
  bool
  operator()(void *lhs, void *rhs) const
  {
    return lhs == rhs;
  }
};

/**
 * @brief Provides storage for all objects known
 * to McMini and keeps track of all object data
 * since the object's creation
 */
class MCObjectStore {
private:

  /**
   * @brief The actual data that is created
   * for each object added into an MCObjectStore
   */
  struct StorageObject final {
    std::shared_ptr<MCVisibleObject> current;
    const std::shared_ptr<MCVisibleObject> initialState;

    StorageObject(std::shared_ptr<MCVisibleObject> current,
                  std::shared_ptr<MCVisibleObject> initialState)
      : current(current), initialState(initialState)
    {}
  };

  /* Points to the most recent item in the storage */
  objid_t storageTop = -1;
  std::shared_ptr<StorageObject>
    storage[MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM];

  /**
   * Maps identities of visible objects given by the system to their
   * shadow-struct counterparts in
   */
  std::unordered_map<MCSystemID, objid_t, PointerHasher,
                     PointersEqual>
    systemVisibleObjectMap;

  inline objid_t
  _registerNewObject(std::shared_ptr<MCVisibleObject> object)
  {
    objid_t newObjectId = ++storageTop;
    MC_ASSERT(newObjectId < MAX_TOTAL_VISIBLE_OBJECTS_IN_PROGRAM);
    object->id              = newObjectId;
    const auto initialState = object->copy();
    storage[newObjectId] =
      std::make_shared<StorageObject>(object, initialState);
    return newObjectId;
  }

public:

  inline MCObjectStore() { bzero(storage, sizeof(storage)); }

  inline objid_t
  registerNewObject(std::shared_ptr<MCVisibleObject> object)
  {
    return this->_registerNewObject(object);
  }

  template<typename Object>
  inline std::shared_ptr<Object>
  getObjectWithId(objid_t id) const
  {
    return std::static_pointer_cast<Object, MCVisibleObject>(
      this->storage[id]->current);
  }

  inline void
  mapSystemAddressToShadow(MCSystemID systemAddress, objid_t shadowId)
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

#endif // MC_MCOBJECTSTORE_H
