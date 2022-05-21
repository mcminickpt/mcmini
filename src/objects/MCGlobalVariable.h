#ifndef MC_MCGLOBALVARIABLE_H
#define MC_MCGLOBALVARIABLE_H

#include "MCVisibleObject.h"

struct MCGlobalVariable : public MCVisibleObject {
    void * const addr;
    explicit MCGlobalVariable(void *addr): addr(addr) {}
    MCGlobalVariable(const MCGlobalVariable &global) : MCGlobalVariable(global.addr) {}

    std::shared_ptr<MCVisibleObject> copy() override;
    MCSystemID getSystemId() override;

    bool operator ==(const MCGlobalVariable&) const;
    bool operator !=(const MCGlobalVariable&) const;
};

#endif //MC_MCGLOBALVARIABLE_H
