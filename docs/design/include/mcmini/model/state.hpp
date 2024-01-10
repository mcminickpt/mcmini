#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/asserts.hpp"
#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {
/**
 * @brief A particular snapshot in time of a program undergoing verification
 * from the perspective of McMini.
 */
class state {
 public:
  using objid_t = uint32_t;

  virtual bool contains_object_with_id(objid_t id) const = 0;
  virtual const visible_object_state *get_state_of_object(objid_t id) const = 0;
  virtual std::unique_ptr<mutable_state> mutable_clone() const = 0;

  // TODO: Provide an interface here that conforms to C++11's
  // iteration (a begin() and end() as virtual functions perhaps).
  // Each subclass can return the same `state_iterator` type that
  // is defined elsewhere which should provide a pair of objid_t
  // and const visible_object_state* associated with that id.

  template <typename StateType, typename ForwardIter, typename... Args>
  static std::unique_ptr<StateType> from_visible_object_states(
      ForwardIter begin, ForwardIter end, Args &&...args) {
    auto state =
        mcmini::extensions::make_unique<StateType>(std::forward<Args>(args)...);
    for (auto elem = begin; elem != end; elem++) {
      state->track_new_visible_object((*elem)->clone());
    }
    return state;
  }

  template <typename StateType, typename ForwardIter, typename... Args>
  static std::unique_ptr<StateType> from_visible_objects(ForwardIter begin,
                                                         ForwardIter end,
                                                         Args &&...args) {
    auto state =
        mcmini::extensions::make_unique<StateType>(std::forward<Args>(args)...);
    for (auto elem = begin; elem != end; elem++) {
      state->track_new_visible_object((*elem).get_current_state()->clone());
    }
    return state;
  }
};

class mutable_state : public state {
  /**
   * @brief Begin tracking a new visible object _obj_ to this state.
   *
   * @param obj the object to begin tracking in this state.
   * @return the new id that is assigned to the object. This id is unique from
   * every other id assigned to the objects in this state.
   */
  virtual objid_t track_new_visible_object(
      std::unique_ptr<some_visible_object> obj) = 0;

  virtual some_visible_object *get_mutable_object_with_id(objid_t id) const = 0;

  /**
   * @brief Adds the given state _state_ for the object with id _id_.
   *
   * Creators of mutable state need to know the contents.
   *
   * @note: If the object with id `id` is _not_ a visible object tracking states
   * of type `visible_object_state_type`, the behavior of this function is
   * undefined.
   */
  template <typename visible_object_state_type>
  void record_new_state_for_visible_object(
      objid_t id, std::unique_ptr<visible_object_state_type> new_state) {
    asserts::assert_condition(
        contains_object_with_id(id),
        "The object must already tracked in order to add a new state");
    auto *assumed_object_type =
        static_cast<visible_object<visible_object_state_type> *>(
            get_mutable_object_with_id(id));
    assumed_object_type.push_state(std::move(new_state));
  }

  /**
   * @brief Creates a copy of the given state.
   *
   * All visible objects in the underlying state are copied into the new state
   * and are independently modifiable with respect to the first state.
   */
  std::unique_ptr<mutable_state> clone() const { return this->mutable_clone(); }
};

}  // namespace mcmini::model