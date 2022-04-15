#ifndef GMAL_GMALVISIBLEOBJECT_H
#define GMAL_GMALVISIBLEOBJECT_H

class GMALState;

#include "GMALShared.h"
#include "GMALVisibleObject.h"

class GMALVisibleObject { objid_t id; GMALSystemID systemId; friend GMALState; };

#endif //GMAL_GMALVISIBLEOBJECT_H
