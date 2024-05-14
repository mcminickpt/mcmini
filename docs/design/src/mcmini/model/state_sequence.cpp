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

//////// diff_state ///////

class state_sequence::diff_state : public mutable_state {
 private:
  const state_sequence &base_state;

 public:
  std::unordered_map<state::objid_t, visible_object> new_object_states;
  std::unordered_map<state::runner_id_t, state::objid_t> new_runners;

  diff_state(const state_sequence &s) : base_state(s) {}
  diff_state(const diff_state &ds)
      : base_state(ds.base_state), new_object_states(ds.new_object_states) {}
  diff_state(detached_state &&) = delete;
  diff_state &operator=(const diff_state &) = delete;
  detached_state &operator=(detached_state &&) = delete;

  /* `state` overrrides */
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
  size_t runner_count() const override {
    return base_state.runner_count() + new_runners.size();
  }
  objid_t get_objid_for_runner(runner_id_t id) const override;
  bool contains_object_with_id(state::objid_t id) const override;
  bool contains_runner_with_id(runner_id_t id) const override;
  const visible_object_state *get_state_of_object(objid_t id) const override;
  const visible_object_state *get_state_of_runner(
      runner_id_t id) const override;
  objid_t add_object(
      std::unique_ptr<const visible_object_state> initial_state) override;
  runner_id_t add_runner(
      std::unique_ptr<const visible_object_state> initial_state) override;
  void add_state_for_obj(
      objid_t id, std::unique_ptr<visible_object_state> new_state) override;
  void add_state_for_runner(
      runner_id_t id, std::unique_ptr<visible_object_state> new_state) override;
  std::unique_ptr<const visible_object_state> consume_obj(objid_t id) &&
      override;
  std::unique_ptr<mutable_state> mutable_clone() const override;
};

std::unique_ptr<mutable_state> state_sequence::mutable_clone() const {
  return extensions::make_unique<diff_state>(*this);
  // return extensions::make_unique<detached_state>(this);
  // return state::from_visible_objects<detached_state>(
  //     this->visible_objects.cbegin(), this->visible_objects.cend());
}

state::objid_t state_sequence::diff_state::get_objid_for_runner(
    runner_id_t id) const {
  if (this->new_runners.count(id) > 0) {
    return this->new_runners.at(id);
  } else {
    return this->base_state.get_objid_for_runner(id);
  }
}

bool state_sequence::diff_state::contains_object_with_id(objid_t id) const {
  return this->new_object_states.count(id) > 0 ||
         this->base_state.contains_object_with_id(id);
}

bool state_sequence::diff_state::contains_runner_with_id(objid_t id) const {
  return this->new_runners.count(id) > 0 ||
         this->base_state.contains_runner_with_id(id);
}

const visible_object_state *state_sequence::diff_state::get_state_of_object(
    objid_t id) const {
  if (this->new_object_states.count(id) > 0) {
    return this->new_object_states.at(id).get_current_state();
  } else {
    return this->base_state.get_state_of_object(id);
  }
}

const visible_object_state *state_sequence::diff_state::get_state_of_runner(
    runner_id_t id) const {
  if (this->new_runners.count(id) > 0) {
    return this->get_state_of_object(this->new_runners.at(id));
  } else {
    return this->base_state.get_state_of_runner(id);
  }
}

state::objid_t state_sequence::diff_state::add_object(
    std::unique_ptr<const visible_object_state> initial_state) {
  // The next id that would be assigned is one more than
  // the largest id available. The last id of the base it `size() - 1` and
  // we are `new_object_state.size()` elements in
  state::objid_t next_id =
      base_state.visible_objects.size() + new_object_states.size();
  new_object_states[next_id] = visible_object(std::move(initial_state));
  return next_id;
}

state::runner_id_t state_sequence::diff_state::add_runner(
    std::unique_ptr<const visible_object_state> initial_state) {
  objid_t objid = this->add_object(std::move(initial_state));

  // The next runner id would be the current size.
  state::objid_t next_runner_id = runner_count();
  this->new_runners.insert({next_runner_id, objid});
  return next_runner_id;
}

void state_sequence::diff_state::add_state_for_obj(
    objid_t id, std::unique_ptr<visible_object_state> new_state) {
  // Here we seek to insert all new states into the local cache instead of
  // forwarding them onto the underlying base state.
  visible_object &vobj = new_object_states[id];
  vobj.push_state(std::move(new_state));
}

void state_sequence::diff_state::add_state_for_runner(
    runner_id_t id, std::unique_ptr<visible_object_state> new_state) {
  this->add_state_for_obj(this->get_objid_for_runner(id), std::move(new_state));
}

std::unique_ptr<const visible_object_state>
state_sequence::diff_state::consume_obj(objid_t id) && {
  throw std::runtime_error("Consumption is not permitted on diff states");
}

std::unique_ptr<mutable_state> state_sequence::diff_state::mutable_clone()
    const {
  return extensions::make_unique<diff_state>(this->base_state);
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