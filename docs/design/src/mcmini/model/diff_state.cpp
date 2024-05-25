#include "mcmini/model/state/diff_state.hpp"

#include <memory>
#include <stdexcept>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/visible_object.hpp"
#include "mcmini/model/visible_object_state.hpp"

using namespace model;

state::objid_t diff_state::get_objid_for_runner(runner_id_t id) const {
  if (this->new_runners.count_domain(id) > 0) {
    return this->new_runners.at(id);
  }
  return this->base_state.get_objid_for_runner(id);
}

runner_id_t diff_state::get_runner_id_for_obj(objid_t id) const {
  if (this->new_runners.count_range(id) > 0) {
    return this->new_runners.range_at(id);
  }
  return this->base_state.get_runner_id_for_obj(id);
}

bool diff_state::is_runner(objid_t id) const {
  return this->new_runners.count_range(id) > 0 ||
         this->base_state.is_runner(id);
}

bool diff_state::contains_object_with_id(objid_t id) const {
  return this->new_object_states.count(id) > 0 ||
         this->base_state.contains_object_with_id(id);
}

bool diff_state::contains_runner_with_id(runner_id_t id) const {
  return this->new_runners.count_domain(id) > 0 ||
         this->base_state.contains_runner_with_id(id);
}

const visible_object_state *diff_state::get_state_of_object(objid_t id) const {
  if (this->new_object_states.count(id) > 0) {
    return this->new_object_states.at(id).get_current_state();
  }
  return this->base_state.get_state_of_object(id);
}

const runner_state *diff_state::get_state_of_runner(runner_id_t id) const {
  if (this->new_runners.count_domain(id) > 0) {
    return static_cast<const runner_state *>(
        this->get_state_of_object(this->new_runners.at(id)));
  }
  return this->base_state.get_state_of_runner(id);
}

state::objid_t diff_state::add_object(
    const visible_object_state *initial_state) {
  // The next id that would be assigned is one more than
  // the largest id available. The last id of the base it `size() - 1` and
  // we are `new_object_state.size()` elements in
  state::objid_t const next_id = base_state.count() + new_object_states.size();
  new_object_states[next_id] = initial_state;
  return next_id;
}

state::runner_id_t diff_state::add_runner(const runner_state *initial_state) {
  objid_t const objid = this->add_object(initial_state);

  // The next runner id would be the current size.
  state::objid_t const next_runner_id = runner_count();
  this->new_runners.insert({next_runner_id, objid});
  return next_runner_id;
}

void diff_state::add_state_for_obj(objid_t id,
                                   const visible_object_state *new_state) {
  if (id == invalid_objid) {
    throw std::runtime_error(
        "Attempted to insert a state for an invalid object id");
  }
  // Here we seek to insert all new states into the local cache instead of
  // forwarding them onto the underlying base state.
  visible_object &vobj = new_object_states[id];
  vobj.push_state(new_state);
}

void diff_state::add_state_for_runner(runner_id_t id,
                                      const runner_state *new_state) {
  this->add_state_for_obj(this->get_objid_for_runner(id), new_state);
}

std::unique_ptr<mutable_state> diff_state::mutable_clone() const {
  throw std::runtime_error("TODO: Implement diff_state cloning");
}
