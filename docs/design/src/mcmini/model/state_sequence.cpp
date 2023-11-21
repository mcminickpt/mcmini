#include "mcmini/model/state/state_sequence.hpp"

#include "mcmini/misc/asserts.hpp"

using namespace mcmini::model;

bool state_sequence::contains_object_with_id(visible_object::objid_t id) const {
  return id < visible_objects.size();
}

visible_object::objid_t state_sequence::track_new_visible_object(
    std::unique_ptr<visible_object_state> initial_state) {
  visible_object obj = visible_object(std::move(initial_state));
  visible_objects.push_back(std::move(obj));
  return visible_objects.size() - 1;
}

void state_sequence::record_new_state_for_visible_object(
    visible_object::objid_t id,
    std::unique_ptr<visible_object_state> next_state) {
  asserts::assert_condition(
      contains_object_with_id(id),
      "The object must already tracked in order to add a new state");
  this->visible_objects.at(id).push_state(std::move(next_state));
}

const visible_object_state &state_sequence::get_state_of_object(
    visible_object::objid_t id) const {
  return *this->visible_objects.at(id).get_current_state();
}

std::unique_ptr<state> state_sequence::clone() const {
  // TODO: Perform a deep copy of the sequence here
  return nullptr;
}

state_sequence state_sequence::consume_into_subsequence(size_t index) && {}
