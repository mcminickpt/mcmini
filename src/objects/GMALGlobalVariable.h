#ifndef GMAL_GMALGLOBALVARIABLE_H
#define GMAL_GMALGLOBALVARIABLE_H

#include "GMALVisibleObject.h"

struct GMALGlobalVariable : public GMALVisibleObject {
    void * const addr;
    explicit GMALGlobalVariable(void *addr): addr(addr) {}
};

#endif //GMAL_GMALGLOBALVARIABLE_H
