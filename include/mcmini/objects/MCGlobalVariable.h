#ifndef MC_MCGLOBALVARIABLE_H
#define MC_MCGLOBALVARIABLE_H

#include "mcmini/objects/MCVisibleObject.h"

struct MCGlobalVariable : public MCVisibleObject {
private:
  inline explicit MCGlobalVariable(void *addr, objid_t id)
    : MCVisibleObject(id), addr(addr)
  {
  }

public:
  void *const addr;
  explicit MCGlobalVariable(void *addr) : addr(addr) {}
  MCGlobalVariable(const MCGlobalVariable &global)
    : MCGlobalVariable(global.addr, global.getObjectId())
  {
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCGlobalVariable &) const;
  bool operator!=(const MCGlobalVariable &) const;
};

#endif // MC_MCGLOBALVARIABLE_H
