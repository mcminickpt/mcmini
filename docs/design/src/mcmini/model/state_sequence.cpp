#include "mcmini/model/state/state_sequence.hpp"

#include "mcmini/misc/asserts.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state/detached_state.hpp"

using namespace mcmini::model;

class state_sequence::diff_state : public mutable_state {
  //  private:
  //   const state &base_state;
  //   std::unordered_map<state::objid_t, visible_object> new_object_states;

  //  public:
  //   diff_state(const state &s) : base_state(s) {}
  //   diff_state(const diff_state &ds)
  //       : base_state(ds.base_state), new_object_states(ds.new_object_states)
  //       {}
  //   diff_state(detached_state &&) = delete;
  //   diff_state &operator=(const diff_state &) = delete;
  //   detached_state &operator=(detached_state &&) = delete;

  // /* `state` overrrides */
  // virtual bool contains_object_with_id(objid_t id) const override;
  // virtual const visible_object_state *get_state_of_object(
  //     objid_t id) const override;
  // virtual objid_t add_object(
  //     std::unique_ptr<visible_object_state> initial_state) override;
  // virtual void add_state_for(
  //     objid_t id, std::unique_ptr<visible_object_state> new_state) override;
  // virtual std::unique_ptr<mutable_state> mutable_clone() const override;
};

state_sequence::state_sequence(const state &initial_state) {
  // TODO: Iterate through all the objects and their states to make a clone
  // Potentially allow for a move iterator to be constructed.
}

state_sequence::state_sequence(const state &&state) {
  // TODO: Iterate through all the objects. We need to attach
}

state_sequence::state_sequence(std::vector<visible_object> &&initial_objects)
    : visible_objects(std::move(initial_objects)) {
  this->states_in_sequence.push_back(state_sequence::element(*this));
}

bool state_sequence::contains_object_with_id(state::objid_t id) const {
  return id < visible_objects.size();
}

const visible_object_state *state_sequence::get_state_of_object(
    objid_t id) const {
  return this->visible_objects.at(id).get_current_state();
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
  // TODO: Initialize this with a reference to this state.
  diff_state *ds;
  auto maybe_diff = t.apply_to(*ds);

  if (maybe_diff.has_value()) {
    const diff_state &ds_after_t =
        static_cast<const diff_state &>(*maybe_diff.value());

    // Apply the diff to the sequence itself and create a new element which
    // refers to the latest states of all the objects. This element is a
    // placeholder for the state of the sequence as it looks after the
    // application of transition `t`. Later if the sequenced is queried for the
    // state of the object.
    this->consume_diff(ds_after_t);
    this->states_in_sequence.push_back(element(*this));

    // consume the difference
    return transition::status::exists;
  }

  return transition::status::disabled;

  // We know the
}

void state_sequence::consume_diff(const diff_state &) {
  /* TODO: Implement consumption of new states */
}

state_sequence state_sequence::consume_into_subsequence(size_t index) && {
  // TODO: Implementation here. The subsequence should look exactly the same
  // as this subsequence up to index `index`.
}

state_sequence::element::element(const state_sequence &owner) {
  for (const auto &obj : owner.visible_objects) {
    this->visible_object_states.push_back(obj.get_current_state());
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

//////// state_sequence::diff_state ///////

// std::unique_ptr<mutable_state> state_sequence::diff_state::mutable_clone()
//     const {
//   return mcmini::extensions::make_unique<diff_state>(this->base_state);
// }