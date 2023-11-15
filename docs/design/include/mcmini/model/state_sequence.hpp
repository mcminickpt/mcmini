#pragma once
#include <string.h>

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {

/**
 * @brief A particular snapshot in time of a program undergoing verification
 * from the perspective of the verifier.
 *
 * A `state` is an element
 */
class state {
  /**
   * @brief
   */
  std::unordered_map<visible_object::objid_t, const visible_object_state *>
      visible_objects;

  /// @brief The sequence within which this state is a part
  ///
  /// A state is a member of state sequence
  const state_sequence *const owning_sequence;

 public:
};

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
class state_sequence {
 private:
  std::unordered_map<visible_object::objid_t, visible_object>
      visible_objects_to_ids;

  std::vector<state> states_in_sequence;

 public:
  // Add a new _distinct_ visible object
  visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object>);

  // Have we assigned a visible object to this id
  bool contains_object_with_id(visible_object::objid_t);

  // Add a new box to a slot
  void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>);

  state_sequence consume_into_subsequence(size_t index) &&;
};

}  // namespace mcmini::model
