#include "mcmini/model/state/state_sequence.hpp"

#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"

using namespace mcmini::model;

state_sequence::state_sequence(const state &initial_state) {
  // Iterate through all the objects and their states to make a clone
  // Potentially allow for a move iterator to be constructed.
}

state_sequence::state_sequence(const state &&state) {
  // Iterate through all the objects. We need to attach
}

state_sequence::state_sequence(
    std::vector<std::unique_ptr<some_visible_object>> &&initial_objects)
    : visible_objects(std::move(initial_objects)) {
  this->states_in_sequence.push_back(state_sequence::element(*this));
}

bool state_sequence::contains_object_with_id(state::objid_t id) const {
  return id < visible_objects.size();
}

const visible_object_state *state_sequence::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id)->get_base_state();
}

std::unique_ptr<mutable_state> state_sequence::mutable_clone() const {
  return state::from_visible_objects<detached_state>(
      this->visible_objects.cbegin(), this->visible_objects.cend());
}

state_sequence state_sequence::consume_into_subsequence(size_t index) && {
  // TODO: Implementation here. The subsequence should look exactly the same
  // as this subsequence up to index `index`.
}

state_sequence::element::element(const state_sequence &owner) {
  for (const auto &obj : owner.visible_objects) {
    this->visible_object_states.push_back(obj->get_base_state());
  }
}

bool state_sequence::element::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_object_states.size();
}

const visible_object_state *state_sequence::element::get_state_of_object(
    state::objid_t id) const {
  return this->visible_object_states.at(id);
}

std::unique_ptr<mutable_state> state_sequence::element::mutable_clone() const {
  return state::from_visible_object_states<detached_state>(
      this->visible_object_states.cbegin(), this->visible_object_states.cend());
}