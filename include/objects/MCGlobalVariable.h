#ifndef MC_MCGLOBALVARIABLE_H
#define MC_MCGLOBALVARIABLE_H

#include "objects/MCVisibleObject.h"

struct MCGlobalVariable : public MCVisibleObject {
private:
  inline explicit MCGlobalVariable(void *addr, char *varName, objid_t id,
                                   uint64_t val)
    : MCVisibleObject(id), addr(addr), varName(varName), val(val)
  {
  }

public:
  void *const addr;
  char *varName;
  uint64_t val;

  explicit MCGlobalVariable(void *addr, char *varName, uint64_t val = 0)
    : addr(addr), varName(varName), val(val) {}

  MCGlobalVariable(const MCGlobalVariable &global)
    : MCGlobalVariable(global.addr, global.varName,
                       global.getObjectId(), global.val)
  {
  }

  std::shared_ptr<MCVisibleObject> copy() override;
  MCSystemID getSystemId() override;
  bool MCObjectEquals(const MCVisibleObject &other) const override;

  bool operator==(const MCGlobalVariable &) const;
  bool operator!=(const MCGlobalVariable &) const;
};

#endif // MC_MCGLOBALVARIABLE_H
