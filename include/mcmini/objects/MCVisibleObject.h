#ifndef MC_MCVISIBLEOBJECT_H
#define MC_MCVISIBLEOBJECT_H

class MCState;
class MCObjectStore;

#include "mcmini/MCShared.h"
#include <memory>

class MCVisibleObject {
  objid_t id;
  friend MCObjectStore;

protected:
  MCVisibleObject() = default;
  MCVisibleObject(objid_t id) : id(id) {}

public:
  virtual std::shared_ptr<MCVisibleObject> copy() = 0;

  virtual MCSystemID getSystemId() = 0;
  objid_t getObjectId() const;
};

#endif // MC_MCVISIBLEOBJECT_H
