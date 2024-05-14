#include "mcmini/model/state/state_sequence.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>

#include "mcmini/misc/append-only.hpp"
#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"

using namespace model;

/**
 * @brief An element of a `model::state_sequence`
 *
 * The `element` and `state_sequence` are tightly intertwined. We allow them
 * to work in tandem with one another as an implementation detail to permit
 * "views" of the objects the state sequence maintains as new states are added
 * to the sequence via transitions
 */
class state_sequence::element : public state {
 private:
  const state_sequence &owner;

  /// @brief A collection of references to states in the sequence
  /// _owning_sequence_ to which this element belongs.
  ///
  /// Each state is a view into a `visible_object` owned by `owner`
  std::unordered_map<state::objid_t, const visible_object_state *>
      visible_object_states;

  // The largest index into the owning state_sequence's runner mapping. That
  // this element is aware of.
  runner_id_t max_visible_runner_id;

  element(const state_sequence &owner);
  friend state_sequence;

 public:
  element() = default;
  void record_new_runner() { max_visible_runner_id++; }
  void point_to_state_for(objid_t id, const visible_object_state *new_state) {
    this->visible_object_states[id] = new_state;
  }
  size_t count() const override { return visible_object_states.size(); }
  size_t runner_count() const override { return max_visible_runner_id; }
  objid_t get_objid_for_runner(runner_id_t id) const override;
  bool contains_object_with_id(state::objid_t id) const override;
  bool contains_runner_with_id(runner_id_t id) const override;
  const visible_object_state *get_state_of_object(objid_t id) const override;
  const visible_object_state *get_state_of_runner(
      runner_id_t id) const override;
  std::unique_ptr<const visible_object_state> consume_obj(objid_t id) &&
      override;
  std::unique_ptr<mutable_state> mutable_clone() const override;
};

state_sequence::state_sequence() { this->push_state_snapshot(); }

state_sequence::state_sequence(const state &s) {
  this->push_state_snapshot();

  // Copy objects
  const size_t num_objs = s.count();
  for (objid_t i = 0; i < num_objs; i++) {
    add_object(s.get_state_of_object(i)->clone());
  }

  // Copy runner information. Here, the state of the runner has already been
  // captured
  this->runner_to_obj_map = append_only<objid_t>(s.runner_count());
  std::iota(this->runner_to_obj_map.begin(), this->runner_to_obj_map.end(), 0);
}

state_sequence::state_sequence(std::vector<visible_object> &&initial_objects)
    : detached_state(std::move(initial_objects)) {
  this->push_state_snapshot();
}

state_sequence::state_sequence(append_only<visible_object> &&ao)
    : detached_state(std::move(ao)) {
  this->push_state_snapshot();
}

void state_sequence::push_state_snapshot() {
  this->states_in_sequence.push_back(new element(*this));
}

state::objid_t state_sequence::add_object(
    std::unique_ptr<const visible_object_state> initial_state) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new object, as this element effectively represents this state
  objid_t id = visible_objects.size();
  this->get_representative_state().point_to_state_for(id, initial_state.get());
  visible_objects.push_back(std::move(initial_state));
  return id;
}

state::runner_id_t state_sequence::add_runner(
    std::unique_ptr<const visible_object_state> initial_state) {
  state::runner_id_t id = detached_state::add_runner(std::move(initial_state));
  this->get_representative_state().record_new_runner();
  return id;
}

void state_sequence::add_state_for_obj(
    objid_t id, std::unique_ptr<visible_object_state> new_state) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new state, as this element effectively represents this state
  this->get_representative_state().point_to_state_for(id, new_state.get());
  this->visible_objects.at(id).push_state(std::move(new_state));
}

void state_sequence::consume_into_subsequence(size_t num_states) {
  if (num_states <= this->states_in_sequence.size())
    this->states_in_sequence.erase(
        this->states_in_sequence.begin() + num_states,
        this->states_in_sequence.end());
}

size_t state_sequence::count() const {
  return this->get_representative_state().count();
}

size_t state_sequence::runner_count() const {
  return this->get_representative_state().runner_count();
}

const state &state_sequence::state_at(size_t i) const {
  return *this->states_in_sequence.at(i);
}

const state &state_sequence::front() const {
  return *this->states_in_sequence.at(0);
}

const state &state_sequence::back() const {
  return *this->states_in_sequence.back();
}

size_t state_sequence::get_num_states_in_sequence() const {
  return this->states_in_sequence.size();
}

state_sequence::~state_sequence() {
  // The elements have been dynamically allocated
  for (const element *element : this->states_in_sequence) delete element;
}

//////// state_sequence::element ///////

state_sequence::element::element(const state_sequence &owner) : owner(owner) {
  for (objid_t i = 0; i < owner.visible_objects.size(); i++) {
    this->visible_object_states[i] =
        owner.visible_objects.at(i).get_current_state();
  }
  this->max_visible_runner_id = owner.runner_to_obj_map.size();
}

state::objid_t state_sequence::element::get_objid_for_runner(
    runner_id_t id) const {
  return this->contains_runner_with_id(id) ? owner.runner_to_obj_map.at(id)
                                           : model::invalid_objid;
}

bool state_sequence::element::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_object_states.size();
}

bool state_sequence::element::contains_runner_with_id(state::objid_t id) const {
  return id < max_visible_runner_id;
}

const visible_object_state *state_sequence::element::get_state_of_object(
    state::objid_t id) const {
  return this->visible_object_states.at(id);
}

const visible_object_state *state_sequence::element::get_state_of_runner(
    runner_id_t id) const {
  return this->get_state_of_object(this->get_objid_for_runner(id));
}

std::unique_ptr<const visible_object_state>
state_sequence::element::consume_obj(objid_t id) && {
  throw std::runtime_error(
      "Consumption is not permitted on elements of a state sequence");
}

std::unique_ptr<mutable_state> state_sequence::element::mutable_clone() const {
  auto state = extensions::make_unique<detached_state>();
  for (objid_t i = 0; i < this->visible_object_states.size(); i++)
    state->add_object(this->visible_object_states.at(i)->clone());
  return state;
}

std::unique_ptr<mutable_state> state_sequence::mutable_clone() const {
  auto detached_ss = extensions::make_unique<detached_state>(*this);
  for (const auto &robjid : runner_to_obj_map) {
    detached_ss->add_runner(get_state_of_object(robjid)->clone());
  }
  return detached_ss;
}

transition::status state_sequence::follow(const transition &t) {
  auto result = t.apply_to(*this);
  if (result.second == model::transition::status::exists) {
    for (auto &new_state : result.first.new_object_states)
      this->visible_objects[new_state.first].push_state(
          new_state.second.consume_into_current_state());
    this->push_state_snapshot();
    return transition::status::exists;
  }
  return transition::status::disabled;
}