#ifndef GMAL_GMALGLOBALVARIABLE_H
#define GMAL_GMALGLOBALVARIABLE_H

#include "GMALVisibleObject.h"

struct GMALGlobalVariable : public GMALVisibleObject {
    void * const addr;
    explicit GMALGlobalVariable(void *addr): addr(addr) {}
    GMALGlobalVariable(const GMALGlobalVariable &global) : GMALGlobalVariable(global.addr) {}

    std::shared_ptr<GMALVisibleObject> copy() override;
    GMALSystemID getSystemId() override;

    bool operator ==(const GMALGlobalVariable&) const;
    bool operator !=(const GMALGlobalVariable&) const;
};

#endif //GMAL_GMALGLOBALVARIABLE_H
