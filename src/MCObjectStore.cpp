#include "mcmini/MCObjectStore.h"

void
MCObjectStore::resetObjectsToInitialStateInStore()
{
  for (objid_t objid = static_cast<objid_t>(0);
       objid <= this->storageTop; objid++) {
    this->storage[objid]->current =
      this->storage[objid]->initialState->copy();
  }
}
