#include "mcmini/model/state/detached_state.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "mcmini/misc/append-only.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/visible_object.hpp"
#include "mcmini/model/visible_object_state.hpp"

using namespace model;

detached_state::detached_state(std::vector<visible_object> &&objs)
    : visible_objects(std::move(objs)) {}
detached_state::detached_state(append_only<visible_object> &&objs)
    : visible_objects(std::move(objs)) {}

bool detached_state::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_objects.size();
}

state::objid_t detached_state::get_objid_for_runner(runner_id_t id) const {
  return this->contains_runner_with_id(id) ? runner_to_obj_map.at(id)
                                           : model::invalid_objid;
}

runner_id_t detached_state::get_runner_id_for_obj(objid_t id) const {
  return this->is_runner(id) ? this->runner_to_obj_map.range_at(id)
                             : model::invalid_rid;
}

bool detached_state::contains_runner_with_id(runner_id_t id) const {
  return this->runner_to_obj_map.count_domain(id) > 0;
}

bool detached_state::is_runner(objid_t id) const {
  return this->runner_to_obj_map.count_range(id) > 0;
}

const visible_object_state *detached_state::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id).get_current_state();
}

const runner_state *detached_state::get_state_of_runner(runner_id_t id) const {
  return static_cast<const runner_state *>(
      this->get_state_of_object(this->get_objid_for_runner(id)));
}

state::objid_t detached_state::add_object(
    const visible_object_state *initial_state) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new object, as this element effectively represents this state
  visible_objects.push_back(initial_state);
  return visible_objects.size() - 1;
}

state::runner_id_t detached_state::add_runner(const runner_state *new_state) {
  const objid_t runner_objid = this->add_object(new_state);
  const runner_id_t next_runner_id = this->runner_to_obj_map.size();
  this->runner_to_obj_map.insert({next_runner_id, runner_objid});
  return next_runner_id;
}

void detached_state::add_state_for_obj(objid_t id,
                                       const visible_object_state *new_state) {
  if (id == invalid_objid) {
    throw std::runtime_error(
        "Attempted to insert a state for an invalid object id");
  }
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new state, as this element effectively represents this state
  this->visible_objects.at(id).push_state(new_state);
}

void detached_state::add_state_for_runner(runner_id_t id,
                                          const runner_state *new_state) {
  return this->add_state_for_obj(this->get_objid_for_runner(id), new_state);
}

std::unique_ptr<mutable_state> detached_state::mutable_clone() const {
  return extensions::make_unique<detached_state>(*this);
}
