#include "mcmini/model/state/detached_state.hpp"

#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"

using namespace model;

detached_state::detached_state(std::vector<visible_object> &&objs)
    : visible_objects(std::move(objs)) {}
detached_state::detached_state(append_only<visible_object> &&objs)
    : visible_objects(std::move(objs)) {}

bool detached_state::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_objects.size();
}

bool detached_state::contains_runner_with_id(state::runner_id_t id) const {
  return id < runner_to_obj_map.size();
}

state::objid_t detached_state::get_objid_for_runner(runner_id_t id) const {
  return this->contains_runner_with_id(id) ? runner_to_obj_map.at(id)
                                           : model::invalid_objid;
}

const visible_object_state *detached_state::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id).get_current_state();
}

const visible_object_state *detached_state::get_state_of_runner(
    runner_id_t id) const {
  return this->get_state_of_object(this->get_objid_for_runner(id));
}

state::objid_t detached_state::add_object(
    std::unique_ptr<const visible_object_state> initial_state) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new object, as this element effectively represents this state
  visible_objects.push_back(std::move(initial_state));
  return visible_objects.size() - 1;
}

state::runner_id_t detached_state::add_runner(
    std::unique_ptr<const visible_object_state> new_state) {
  objid_t id = this->add_object(std::move(new_state));
  this->runner_to_obj_map.push_back(id);
  return this->runner_to_obj_map.size() - 1;
}

void detached_state::add_state_for_obj(
    objid_t id, std::unique_ptr<visible_object_state> new_state) {
  if (id == invalid_objid) {
    throw std::runtime_error("Attempted to insert an invalid object id");
  }
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new state, as this element effectively represents this state
  this->visible_objects.at(id).push_state(new_state.release());
}

void detached_state::add_state_for_runner(
    runner_id_t id, std::unique_ptr<visible_object_state> new_state) {
  return this->add_state_for_obj(this->get_objid_for_runner(id),
                                 std::move(new_state));
}

std::unique_ptr<const visible_object_state> detached_state::consume_obj(
    objid_t id) && {
  return std::move(visible_objects.at(id)).consume_into_current_state();
}

std::unique_ptr<mutable_state> detached_state::mutable_clone() const {
  return extensions::make_unique<detached_state>(*this);
}