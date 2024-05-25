#include "mcmini/model/state/state_sequence.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mcmini/misc/append-only.hpp"
#include "mcmini/misc/extensions/memory.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/visible_object.hpp"
#include "mcmini/model/visible_object_state.hpp"

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
  const state_sequence *owner;

  /// @brief A collection of references to states in the sequence
  /// _owning_sequence_ to which this element belongs.
  ///
  /// Each state is a view into a `visible_object` owned by `owner`
  std::unordered_map<state::objid_t, const visible_object_state *>
      visible_object_states;

  // The largest index into the owning state_sequence's runner mapping. That
  // this element is aware of.
  runner_id_t max_visible_runner_id;

  element(const state_sequence *owner);
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
  runner_id_t get_runner_id_for_obj(objid_t id) const override;
  bool is_runner(objid_t id) const override;
  bool contains_object_with_id(state::objid_t id) const override;
  bool contains_runner_with_id(runner_id_t id) const override;
  const visible_object_state *get_state_of_object(objid_t id) const override;
  const runner_state *get_state_of_runner(runner_id_t id) const override;
  std::unique_ptr<mutable_state> mutable_clone() const override;
};

state_sequence::state_sequence() { this->push_state_snapshot(); }

state_sequence::state_sequence(const state &s) {
  this->push_state_snapshot();
  const size_t num_objs = s.count();
  for (objid_t i = 0; i < (objid_t)num_objs; i++)
    add_object(s.get_state_of_object(i)->clone().release());

  const size_t num_runners = s.runner_count();
  for (runner_id_t p = 0; p < (runner_id_t)num_runners; p++) {
    this->get_representative_state().record_new_runner();
    this->runner_to_obj_map.insert({p, s.get_objid_for_runner(p)});
  }
}

state_sequence::state_sequence(state_sequence &&s) {
  // For the std::vector<> move constructor:
  // """
  // Constructs the container with the contents of
  // other using move semantics. Allocator is obtained by move-construction
  // from the allocator belonging to other. After the move, other is guaranteed
  // to be empty().
  // """
  // See https://en.cppreference.com/w/cpp/container/vector/vector
  this->states_in_sequence = std::move(s.states_in_sequence);
  this->visible_objects = std::move(s.visible_objects);
  this->runner_to_obj_map = std::move(s.runner_to_obj_map);

  // The elements currently point to `s` as their owner. Since we
  // are now transferring ownership to _this_ state object, we need to inform
  // each element. Note that the states they refer to are still valid however as
  // we also acquire the visible objects they refer to viz. `s.visible_objects`
  for (element *s : this->states_in_sequence) s->owner = this;
}

state_sequence::state_sequence(std::vector<visible_object> &&initial_objects)
    : visible_objects(std::move(initial_objects)) {
  this->push_state_snapshot();
}

state_sequence::state_sequence(append_only<visible_object> &&ao)
    : visible_objects(std::move(ao)) {
  this->push_state_snapshot();
}

void state_sequence::push_state_snapshot() {
  this->states_in_sequence.push_back(new element(this));
}

bool state_sequence::is_runner(objid_t id) const {
  return this->get_representative_state().is_runner(id);
}

state::objid_t state_sequence::get_objid_for_runner(runner_id_t id) const {
  return this->get_representative_state().get_objid_for_runner(id);
}

runner_id_t state_sequence::get_runner_id_for_obj(objid_t id) const {
  return this->get_representative_state().get_runner_id_for_obj(id);
}

bool state_sequence::contains_object_with_id(objid_t id) const {
  return this->get_representative_state().contains_object_with_id(id);
}

bool state_sequence::contains_runner_with_id(runner_id_t id) const {
  return this->get_representative_state().contains_runner_with_id(id);
}

const visible_object_state *state_sequence::get_state_of_object(
    objid_t id) const {
  return this->get_representative_state().get_state_of_object(id);
}

const runner_state *state_sequence::get_state_of_runner(runner_id_t id) const {
  return this->get_representative_state().get_state_of_runner(id);
}

state::objid_t state_sequence::add_object(const visible_object_state *s) {
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new object, as this element effectively represents this state
  objid_t const id = visible_objects.size();
  this->get_representative_state().point_to_state_for(id, s);
  visible_objects.push_back(s);
  return id;
}

state::runner_id_t state_sequence::add_runner(const runner_state *s) {
  const objid_t runner_objid = this->add_object(s);
  const runner_id_t next_runner_id = this->runner_to_obj_map.size();
  this->runner_to_obj_map.insert({next_runner_id, runner_objid});
  this->get_representative_state().record_new_runner();
  return next_runner_id;
}

void state_sequence::add_state_for_obj(objid_t id,
                                       const visible_object_state *new_state) {
  if (id == invalid_objid) {
    throw std::runtime_error(
        "Attempted to insert a state for an invalid object id");
  }
  // INVARIANT: The current element needs to update at index `id` to reflect
  // this new state, as this element effectively represents this state
  this->get_representative_state().point_to_state_for(id, new_state);
  this->visible_objects.at(id).push_state(new_state);
}

void state_sequence::add_state_for_runner(runner_id_t id,
                                          const runner_state *new_state) {
  objid_t const objid = this->get_objid_for_runner(id);
  if (objid == invalid_objid) {
    throw std::runtime_error(
        "Attempted to insert a state for a runner that does not exist in "
        "this state.");
  }
  this->add_state_for_obj(objid, new_state);
}

void state_sequence::consume_into_subsequence(size_t num_states) {
  if (num_states <= this->states_in_sequence.size()) {
    extensions::destroy(this->states_in_sequence.begin() + num_states,
                        this->states_in_sequence.end());
    this->states_in_sequence.erase(
        this->states_in_sequence.begin() + num_states,
        this->states_in_sequence.end());
  }
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

state_sequence::element::element(const state_sequence *owner) : owner(owner) {
  for (objid_t i = 0; i < owner->visible_objects.size(); i++) {
    this->visible_object_states[i] =
        owner->visible_objects.at(i).get_current_state();
  }
  this->max_visible_runner_id = owner->runner_to_obj_map.size();
}

runner_id_t state_sequence::element::get_runner_id_for_obj(objid_t id) const {
  return this->is_runner(id) ? owner->runner_to_obj_map.range_at(id)
                             : model::invalid_rid;
}

bool state_sequence::element::is_runner(objid_t id) const {
  return this->contains_object_with_id(id) &&
         owner->runner_to_obj_map.count_range(id) > 0;
}

state::objid_t state_sequence::element::get_objid_for_runner(
    runner_id_t id) const {
  return this->contains_runner_with_id(id) ? owner->runner_to_obj_map.at(id)
                                           : model::invalid_objid;
}

bool state_sequence::element::contains_object_with_id(state::objid_t id) const {
  return id < this->visible_object_states.size();
}

bool state_sequence::element::contains_runner_with_id(
    state::runner_id_t id) const {
  return id < max_visible_runner_id &&
         owner->runner_to_obj_map.count_domain(id) > 0;
}

const visible_object_state *state_sequence::element::get_state_of_object(
    state::objid_t id) const {
  return this->visible_object_states.at(id);
}

const runner_state *state_sequence::element::get_state_of_runner(
    runner_id_t id) const {
  return static_cast<const runner_state *>(
      this->get_state_of_object(this->get_objid_for_runner(id)));
}

std::unique_ptr<mutable_state> state_sequence::element::mutable_clone() const {
  auto state = extensions::make_unique<detached_state>();
  for (objid_t i = 0; i < this->visible_object_states.size(); i++)
    state->add_object(this->visible_object_states.at(i)->clone().release());
  return state;
}

std::unique_ptr<mutable_state> state_sequence::mutable_clone() const {
  // auto detached_ss = extensions::make_unique<detached_state>(*this);
  // for (const auto &robjid : runner_to_obj_map) {
  //   detached_ss->add_runner(get_state_of_object(robjid)->clone().release());
  // }
  // return detached_ss;
  throw std::runtime_error("TODO: Implement state cloning for state_sequence");
}

transition::status state_sequence::follow(const transition &t) {
  auto result = t.apply_to(*this);
  if (result.second == model::transition::status::exists) {
    for (auto &new_state : result.first.new_object_states)
      this->visible_objects[new_state.first].push_state(
          new_state.second.consume_into_current_state().release());
    this->push_state_snapshot();
    return transition::status::exists;
  }
  return transition::status::disabled;
}
