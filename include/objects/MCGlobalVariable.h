#ifndef MC_MCGLOBALVARIABLE_H
#define MC_MCGLOBALVARIABLE_H

#include "objects/MCVisibleObject.h"

struct MCGlobalVariable : public MCVisibleObject {
private:
  inline explicit MCGlobalVariable(void *addr, char *varName, objid_t id)
    : MCVisibleObject(id), addr(addr), varName(varName)
  {
  }

public:
  void *const addr;
  char *varName;
  explicit MCGlobalVariable(void *addr, char *varName) : 
    addr(addr), varName(varName) {}

  MCGlobalVariable(const MCGlobalVariable &global)
    : MCGlobalVariable(global.addr, global.varName, global.getObjectId())
  {
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;

  bool operator==(const MCGlobalVariable &) const;
  bool operator!=(const MCGlobalVariable &) const;
};

#endif // MC_MCGLOBALVARIABLE_H
