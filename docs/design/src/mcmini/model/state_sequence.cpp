#include "mcmini/model/state_sequence.hpp"

using namespace mcmini::model;

bool state_sequence::contains_object_with_id(visible_object::objid_t id) const {
    return }

visible_object::objid_t state_sequence::track_new_visible_object(
    std::unique_ptr<visible_object_state>) {
  //   this->owning_sequence->track_new_visible_object();
  return 0;
}

void state_sequence::record_new_state_for_visible_object(
    visible_object::objid_t, std::unique_ptr<visible_object_state>) {}

const visible_object_state *state_sequence::get_state_of_object(
    visible_object::objid_t) const {
  return nullptr;
}

std::unique_ptr<state> state_sequence::clone() const {
  // TODO: Perform a deep copy of the sequence here
  return nullptr;
}