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
 private:
  append_only<model::visible_object> visible_objects;

 public:
  detached_state() = default;
  detached_state(const detached_state &) = default;
  detached_state(detached_state &&) = default;
  detached_state &operator=(const detached_state &) = default;
  detached_state &operator=(detached_state &&) = default;

  /* `state` overrrides */
  size_t count() const override { return visible_objects.size(); }
  bool contains_object_with_id(objid_t id) const override;
  const visible_object_state *get_state_of_object(objid_t id) const override;
  objid_t add_object(
      std::unique_ptr<const visible_object_state> initial_state) override;
  void add_state_for(objid_t id,
                     std::unique_ptr<visible_object_state> new_state) override;
  std::unique_ptr<const visible_object_state> consume_obj(objid_t id) &&
      override;
  std::unique_ptr<mutable_state> mutable_clone() const override;
};

}  // namespace model