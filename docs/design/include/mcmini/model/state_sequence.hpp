#pragma once
#include <string.h>

#include <memory>
#include <unordered_map>

#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {

/**
 *
 */
class state {
  std::unordered_map<objid_t, visible_object_state> visible_objects;

 public:
};

/**
 * @brief A sequence of a states.
 *
 * A _state sequence_ is a sequence of
 *
 * A _state_ is an unordered container of visible objects. Each visible object
 * is in turn composed of a sequence of
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
  std::unordered_map<objid_t, visible_object> visible_objects_to_ids;

  std::vector<state> states_in_sequence;

 public:
  // Add a new _distinct_ visible object
  objid_t track_new_visible_object(std::unique_ptr<visible_object>);
  objid_t track_new_visible_object(std::unique_ptr<visible_object_state>);

  // Have we assigned a visible object to this id
  bool does_visible_object_exist(objid_t);

  // Add a new box to a slot
  void record_new_state_for_visible_object(
      objid_t, std::unique_ptr<visible_object_state>);

  state_sequence consume_into_subsequence(size_t index) &&;
};

}  // namespace mcmini::model
