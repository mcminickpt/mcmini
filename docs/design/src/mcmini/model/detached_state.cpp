#include "mcmini/model/state/detached_state.hpp"

#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"

using namespace mcmini::model;

/* `state` overrrides */
bool detached_state::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_objects.size();
}

state::objid_t detached_state::track_new_visible_object(
    std::unique_ptr<some_visible_object> new_object) {
  visible_objects.push_back(std::move(new_object));
  return visible_objects.size() - 1;
}

const visible_object_state *detached_state::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id)->get_base_state();
}

some_visible_object *detached_state::get_mutable_object_with_id(
    objid_t id) const {
  return this->visible_objects.at(id).get();
}

std::unique_ptr<mutable_state> detached_state::mutable_clone() const {
  // return mcmini::extensions::make_unique<detached_state>(*this);
  return nullptr;
}