#include "mcmini/model/state/state_sequence.hpp"

#include <iterator>

#include "mcmini/misc/append-only.hpp"
#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"

using namespace model;

class state_sequence::diff_state : public mutable_state {
 private:
  const state_sequence &base_state;

 public:
  // Purposely exposed in this private cpp file for implementation (pimpl).
  std::unordered_map<state::objid_t, visible_object> new_object_states;

  diff_state(const state_sequence &s) : base_state(s) {}
  diff_state(const diff_state &ds)
      : base_state(ds.base_state), new_object_states(ds.new_object_states) {}
  diff_state(detached_state &&) = delete;
  diff_state &operator=(const diff_state &) = delete;
  detached_state &operator=(detached_state &&) = delete;

  /* `state` overrrides */
  virtual bool contains_object_with_id(objid_t id) const override;
  virtual size_t count() const override {
    size_t count = this->new_object_states.size() + base_state.count();
    for (const auto p : new_object_states) {
      // Each item in `new_object_states` that is also in `base_state` defines
      // states for _previously existing_ objects. These objects are accounted
      // for in `count` (double-counted), hence the `--`
      if (base_state.contains_object_with_id(p.first)) count--;
    }
    return count;
  }
  const visible_object_state *get_state_of_object(objid_t id) const override;
  objid_t add_object(
      std::unique_ptr<const visible_object_state> initial_state) override;
  void add_state_for(objid_t id,
                     std::unique_ptr<visible_object_state> new_state) override;
  std::unique_ptr<const visible_object_state> consume_obj(objid_t id) &&
      override;
  std::unique_ptr<mutable_state> mutable_clone() const override;
};

state_sequence::state_sequence() { this->push_state_snapshot(); }

state_sequence::state_sequence(const state &s) {
  this->push_state_snapshot();
  const size_t num_objs = s.count();
  for (objid_t i = 0; i < num_objs; i++)
    add_object(s.get_state_of_object(i)->clone());
}

state_sequence::state_sequence(std::vector<visible_object> &&initial_objects)
    : visible_objects(std::move(initial_objects)) {
  this->push_state_snapshot();
}

state_sequence::state_sequence(append_only<visible_object> &&ao)
    : visible_objects(std::move(ao)) {
  this->push_state_snapshot();
}

bool state_sequence::contains_object_with_id(state::objid_t id) const {
  return id < visible_objects.size();
}

const visible_object_state *state_sequence::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id).get_current_state();
}

state::objid_t state_sequence::add_object(
    std::unique_ptr<const visible_object_state> initial_state) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new object, as this element effectively represents this state
  objid_t id = visible_objects.size();
  this->states_in_sequence.back().point_to_state_for(id, initial_state.get());
  visible_objects.push_back(std::move(initial_state));
  return id;
}

void state_sequence::add_state_for(
    objid_t id, std::unique_ptr<visible_object_state> new_state) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new state, as this element effectively represents this state
  this->states_in_sequence.back().point_to_state_for(id, new_state.get());
  this->visible_objects.at(id).push_state(std::move(new_state));
}

std::unique_ptr<const visible_object_state> state_sequence::consume_obj(
    objid_t id) && {
  return std::move(visible_objects.at(id)).consume_into_current_state();
}

std::unique_ptr<mutable_state> state_sequence::mutable_clone() const {
  return state::from_visible_objects<detached_state>(
      this->visible_objects.cbegin(), this->visible_objects.cend());
}

transition::status state_sequence::follow(const transition &t) {
  // Supply the transition a `diff_state` intentionally. We have control over
  // the `apply_to` function: it simply returns a clone of the state it was
  // provided. A `diff_state` copies only the reference to its underlying
  // "backing" state, and only new objects will be
  diff_state ds(*this);
  auto maybe_diff = t.apply_to(ds);

  if (maybe_diff != nullptr) {
    // Apply the diff to the sequence itself and create a new element which
    // refers to the latest states of all the objects. The element is a
    // placeholder for the state of the sequence as it looks after the
    // application of transition `t`. Later if the sequenced is queried for
    // the state of the object.
    //
    // Assumes that the `mutable_clone()` of diff_state is also a
    // `diff_state` and that `apply_to()` returns the mutated clone. Since these
    // are implementation details under our control, this assumption holds
    const diff_state &ds_after_t = static_cast<const diff_state &>(*maybe_diff);

    for (auto new_state : ds_after_t.new_object_states)
      this->visible_objects[new_state.first].push_state(
          std::move(new_state.second).consume_into_current_state());

    this->push_state_snapshot();
    return transition::status::exists;
  }
  return transition::status::disabled;
}

state_sequence state_sequence::consume_into_subsequence(size_t index) && {
  auto elements = append_only<state_sequence::element>(
      std::make_move_iterator(this->states_in_sequence.begin()),
      std::make_move_iterator(this->states_in_sequence.begin() + index + 1));
  auto ss = state_sequence(std::move(this->visible_objects));
  ss.states_in_sequence = std::move(elements);
  return ss;
}

//////// state_sequence::element ///////

state_sequence::element::element(const state_sequence &owner) {
  for (objid_t i = 0; i < owner.visible_objects.size(); i++) {
    this->visible_object_states[i] =
        owner.visible_objects.at(i).get_current_state();
  }
}

bool state_sequence::element::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_object_states.size();
}

const visible_object_state *state_sequence::element::get_state_of_object(
    state::objid_t id) const {
  return this->visible_object_states.at(id);
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

//////// diff_state ///////

bool state_sequence::diff_state::contains_object_with_id(objid_t id) const {
  return this->new_object_states.count(id) > 0 ||
         this->base_state.contains_object_with_id(id);
}

const visible_object_state *state_sequence::diff_state::get_state_of_object(
    objid_t id) const {
  if (this->new_object_states.count(id) > 0) {
    return this->new_object_states.at(id).get_current_state();
  } else {
    return this->base_state.get_state_of_object(id);
  }
}

state::objid_t state_sequence::diff_state::add_object(
    std::unique_ptr<const visible_object_state> initial_state) {
  // The next id that would be assigned if one more than
  // the largest id available. The last id of the base it `size() - 1` and
  // we are `new_object_state.size()` elements in
  state::objid_t next_id =
      base_state.visible_objects.size() + new_object_states.size();
  new_object_states[next_id] = visible_object(std::move(initial_state));
  return next_id;
}

void state_sequence::diff_state::add_state_for(
    objid_t id, std::unique_ptr<visible_object_state> new_state) {
  auto &s = new_object_states[id];
  s.push_state(std::move(new_state));
}

std::unique_ptr<const visible_object_state>
state_sequence::diff_state::consume_obj(objid_t id) && {
  throw std::runtime_error("Consumption is not permitted on diff states");
}

std::unique_ptr<mutable_state> state_sequence::diff_state::mutable_clone()
    const {
  return extensions::make_unique<diff_state>(this->base_state);
}