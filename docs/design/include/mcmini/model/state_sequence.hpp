#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/model/state.hpp"

namespace mcmini::model {

/**
 * @brief A sequence of a state.
 *
 * A _state sequence_ describes how a `mcnini::model:;program`'s state has
 * changed over time as its execution units have executed and taken action.
 *
 * A _state_ is an unordered container of visible objects.
 * Each visible object is in turn composed of a sequence of
 *
 *
 * * The sequence "owns" the visible objects
 * * A visible object is effectively objid_t
 * * Each visible object is effectively a sequence of states
 * * Three operations we just talked about
 * * The
 */
class state_sequence : public state {
 private:
  std::unordered_map<visible_object::objid_t, visible_object>
      visible_objects_to_ids;
  std::vector<state_view> states_in_sequence;

 public:
  /* `state` overrrides */
  virtual bool contains_object_with_id(
      visible_object::objid_t id) const override;
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state>) override;
  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) override;
  virtual const visible_object_state *get_state_of_object(
      visible_object::objid_t) const override;
  virtual std::unique_ptr<state> clone() const override;

 public:
  const state_view &state_at(size_t i) const {
    return this->states_in_sequence.at(i);
  }
  /**
   *
   */
  state_sequence consume_into_subsequence(size_t index) &&;
};

}  // namespace mcmini::model
