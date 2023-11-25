#pragma once

#include "mcmini/misc/append-only.hpp"
#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/state.hpp"

namespace mcmini::model {

/**
 * @brief A collection of visible object states.
 *
 * A `detached_state` is one which defines a state of a program outside of the
 * context of a sequence; that is, a detached state represents the states
 */
class detached_state : public mutable_state {
 private:
  mcmini::append_only<visible_object> visible_objects;

 public:
  detached_state() = default;
  detached_state(const detached_state &) = default;
  detached_state(detached_state &&) = default;
  detached_state &operator=(const detached_state &) = default;
  detached_state &operator=(detached_state &&) = default;

  template <typename ForwardIter>
  static std::unique_ptr<detached_state> from_objects(ForwardIter begin,
                                                      ForwardIter end) {
    auto state = mcmini::extensions::make_unique<detached_state>();
    for (auto elem = begin; elem != end; elem++) {
      state->track_new_visible_object((*elem).get_current_state()->clone());
    }
    return state;
  }

  template <typename ForwardIter>
  static std::unique_ptr<detached_state> from_states(ForwardIter begin,
                                                     ForwardIter end) {
    auto state = mcmini::extensions::make_unique<detached_state>();
    for (auto elem = begin; elem != end; elem++) {
      state->track_new_visible_object((*elem)->clone());
    }
    return state;
  }

  /* `state` overrrides */
  virtual bool contains_object_with_id(
      visible_object::objid_t id) const override;
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state>) override;
  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) override;
  virtual const visible_object_state &get_state_of_object(
      visible_object::objid_t) const override;
  virtual std::unique_ptr<mutable_state> mutable_clone() const override;
};

}  // namespace mcmini::model