#include "mcmini/model/state_sequence.hpp"

#include "mcmini/model/state.hpp"

using namespace mcmini::model;

bool state_view::contains_object_with_id(visible_object::objid_t id) const {
  return this->owning_sequence->contains_object_with_id(id);
}

visible_object::objid_t state_view::track_new_visible_object(
    std::unique_ptr<visible_object_state>) {
  //   this->owning_sequence->track_new_visible_object();
  return 0;
}

void state_view::record_new_state_for_visible_object(
    visible_object::objid_t, std::unique_ptr<visible_object_state>) {}
const visible_object_state *state_view::get_state_of_object(
    visible_object::objid_t) const {
  return nullptr;
}

std::unique_ptr<state> state_view::clone() {
  return std::unique_ptr<state_view>(new state_view(this->owning_sequence));
}