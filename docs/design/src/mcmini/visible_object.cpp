#include "mcmini/model/visible_object.hpp"

using namespace model;

visible_object::~visible_object() {
  for (const visible_object_state* s : history)
    if (s) delete s;
}