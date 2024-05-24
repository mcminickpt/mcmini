#include "mcmini/model/visible_object.hpp"

#include "mcmini/model/visible_object_state.hpp"

using namespace model;

visible_object::~visible_object() {
  for (const visible_object_state* s : history)
    if (s) delete s;
}
