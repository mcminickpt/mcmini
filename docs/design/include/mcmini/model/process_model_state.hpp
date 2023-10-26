#pragma once
#include <string.h>

#include <memory>
#include <unordered_map>

#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {

/**
 *
 *
 */
class process_model_state {
 public:
  inline objid_t registerNewObject(std::shared_ptr<MCVisibleObject> object) {
    return this->_registerNewObject(object);
  }

  template <typename ConcreteObject>
  inline const ConcreteObject *get_object_with_id(visible_object::id id) const {
    return static_cast<ConcreteObject *>(this->storage[id]->current);
  }

  inline void mapSystemAddressToShadow(MCSystemID systemAddress,
                                       objid_t shadowId) {
    systemVisibleObjectMap.insert({systemAddress, shadowId});
  }

  template <typename Object>
  inline std::shared_ptr<Object> getObjectWithSystemAddress(
      void *systemAddress) {
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

}  // namespace mcmini::model
