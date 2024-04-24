#pragma once

#include "mcmini/misc/append-only.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/visible_object.hpp"

namespace model {

/**
 * @brief A collection of visible object states.
 *
 * A `detached_state` is one which defines a state of a program outside of the
 * context of a sequence; that is, a detached state represents the states
 */
class detached_state : public model::mutable_state {
 protected:
  append_only<model::visible_object> visible_objects;

  // INVARIANT: Runner ids are assigned sequentially. A runner with id `id` is
  // mapped to the object id at index `id - 1`.
  append_only<state::objid_t> runner_to_obj_map;

 public:
  detached_state() = default;
  detached_state(std::vector<visible_object> &&);
  detached_state(append_only<visible_object> &&);
  detached_state(const detached_state &) = default;
  detached_state(detached_state &&) = default;
  detached_state &operator=(const detached_state &) = default;
  detached_state &operator=(detached_state &&) = default;

  /* `state` overrrides */
  size_t count() const override { return visible_objects.size(); }
  size_t runner_count() const override { return runner_to_obj_map.size(); }
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

}  // namespace model