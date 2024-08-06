#include "mcmini/model/visible_object.hpp"

#include "mcmini/misc/extensions/memory.hpp"

using namespace model;

void visible_object::slice(size_t index) {
  extensions::delete_all(this->history.begin() + index + 1,
                         this->history.end());
  this->history.erase(this->history.begin() + index + 1, this->history.end());
}

visible_object::~visible_object() {
  for (const visible_object_state* s : history) delete s;
}
