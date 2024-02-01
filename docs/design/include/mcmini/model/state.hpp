#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/asserts.hpp"
#include "mcmini/model/visible_object.hpp"

namespace model {
/**
 * @brief A particular snapshot in time of a program undergoing verification
 * from the perspective of McMini.
 */
class state {
 public:
  using objid_t = uint32_t;

  virtual ~state() = default;
  virtual bool contains_object_with_id(objid_t id) const = 0;
  virtual const visible_object_state *get_state_of_object(objid_t id) const = 0;
  virtual std::unique_ptr<mutable_state> mutable_clone() const = 0;

  // TODO: Potentially provide an interface here that conforms to C++11's
  // iteration (a begin() and end() as virtual functions perhaps).
  // Each subclass can return the same `state_iterator` type that
  // is defined elsewhere which should provide a pair of objid_t
  // and const visible_object_state* associated with that id.

  template <typename StateType, typename ForwardIter, typename... Args>
  static std::unique_ptr<StateType> from_visible_object_states(
      ForwardIter begin, ForwardIter end, Args &&...args) {
    auto state =
        extensions::make_unique<StateType>(std::forward<Args>(args)...);
    for (auto elem = begin; elem != end; elem++) {
      state->add_object((*elem)->clone());
    }
    return state;
  }

  template <typename StateType, typename ForwardIter, typename... Args>
  static std::unique_ptr<StateType> from_visible_objects(ForwardIter begin,
                                                         ForwardIter end,
                                                         Args &&...args) {
    auto state =
        extensions::make_unique<StateType>(std::forward<Args>(args)...);
    for (auto elem = begin; elem != end; elem++) {
      state->add_object((*elem).get_current_state()->clone());
    }
    return state;
  }

  template <typename concrete_visible_object_state>
  const concrete_visible_object_state *get_state_of_object(objid_t id) const {
    static_assert(std::is_base_of<visible_object_state,
                                  concrete_visible_object_state>::value,
                  "Concrete type must be a subtype of `visible_object_state`");
    return static_cast<const concrete_visible_object_state *>(
        this->get_state_of_object(id));
  }
};

class mutable_state : public state {
 public:
  virtual ~mutable_state() = default;

  /**
   * @brief Begin tracking a new visible object _obj_ to this state.
   *
   * @param initial_state the initial state of the object
   * @return the new id that is assigned to the object. This id is unique from
   * every other id assigned to the objects in this state.
   */
  virtual objid_t add_object(
      std::unique_ptr<visible_object_state> initial_state) = 0;

  /**
   * @brief Adds the given state _state_ for the object with id _id_.
   *
   * @note: If the object with id `id` is _not_ a visible object tracking states
   * of type `visible_object_state_type`, the behavior of this function is
   * undefined.
   */
  virtual void add_state_for(
      objid_t id, std::unique_ptr<visible_object_state> new_state) = 0;

  /**
   * @brief Creates a copy of the given state.
   *
   * All visible objects in the underlying state are copied into the new state
   * and are independently modifiable with respect to the first state.
   */
  std::unique_ptr<mutable_state> clone() const { return this->mutable_clone(); }

  template <typename concrete_visible_object_state>
  const concrete_visible_object_state *get_state_of_object(objid_t id) const {
    return (static_cast<const state *>(this))
        ->get_state_of_object<concrete_visible_object_state>();
  }
};

}  // namespace model