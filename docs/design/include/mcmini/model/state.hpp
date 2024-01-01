#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
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
  virtual const visible_object_state &get_object_with_id(objid_t id) const = 0;
  virtual std::unique_ptr<mutable_state> mutable_clone() const = 0;

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
      std::unique_ptr<visible_object_base> obj) = 0;

  /**
   * @brief Adds the given state _state_ for the object with id _id_.
   */
  virtual void record_new_state_for_visible_object(
      objid_t, std::unique_ptr<visible_object_state>) = 0;

  /**
   * @brief Creates a copy of the given state.
   *
   * All visible objects in the underlying state are copied into the new state
   * and are independently modifiable with respect to the first state.
   */
  std::unique_ptr<mutable_state> clone() const { return this->mutable_clone(); }
};

}  // namespace mcmini::model