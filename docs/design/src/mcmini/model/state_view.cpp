#include "mcmini/model/state/state_view.hpp"

#include "mcmini/model/state/state_sequence.hpp"

using namespace mcmini::model;

state_view::state_view(state_sequence &owner) : owning_sequence(owner) {
  for (const visible_object &obj : owner.visible_objects) {
    this->visible_object_states.push_back(obj.get_current_state());
  }
}

bool state_view::contains_object_with_id(visible_object::objid_t id) const {
  return this->owning_sequence.contains_object_with_id(id);
}

visible_object::objid_t state_view::track_new_visible_object(
    std::unique_ptr<visible_object_state> initial_state) {
  return this->owning_sequence.track_new_visible_object(
      std::move(initial_state));
}

void state_view::record_new_state_for_visible_object(
    visible_object::objid_t id,
    std::unique_ptr<visible_object_state> new_state) {
  this->owning_sequence.record_new_state_for_visible_object(
      id, std::move(new_state));
}

const visible_object_state &state_view::get_state_of_object(
    visible_object::objid_t id) const {
  return *this->visible_object_states.at(id);
}

std::unique_ptr<state> state_view::clone() const {
  return std::unique_ptr<state_view>(new state_view(this->owning_sequence));
}