#include "mcmini/model/state/detached_state.hpp"

#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"

using namespace mcmini::model;

/* `state` overrrides */
bool detached_state::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_objects.size();
}

state::objid_t detached_state::add_object(
    std::unique_ptr<visible_object_state> new_object) {
  visible_objects.push_back(visible_object(std::move(new_object)));
  return visible_objects.size() - 1;
}

void detached_state::add_state_for(
    objid_t id, std::unique_ptr<visible_object_state> new_state) {
  this->visible_objects.at(id).push_state(std::move(new_state));
}

const visible_object_state *detached_state::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id).get_current_state();
}

std::unique_ptr<mutable_state> detached_state::mutable_clone() const {
  return mcmini::extensions::make_unique<detached_state>(*this);
}