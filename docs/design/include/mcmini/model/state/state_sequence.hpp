#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/state/state_view.hpp"

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
  // INVARIANT: As new states are added to the visible objects in the
  // mapping, new state views are also added with the appropriate object states
  // replaced.

  // INVARIANT: Objects must only be added to the collection and are never
  // removed.
  //
  // TODO: In the future, if there is a collection which is "append-only",
  // that would be preferred as this would prevent at compile-time attempting to
  // add elements into the vector.
  std::vector<visible_object> visible_objects;
  std::vector<state_view> states_in_sequence;
  friend state_view; /* The state_view and state_sequence are tightly
                        intertwined. We allow them to work in tandem with one
                        another as an implementation detail */

 public:
  state_sequence &operator=(const state_sequence &&) = delete;
  state_sequence &operator=(const state_sequence &) = delete;
  state_sequence(state_sequence &) = delete;
  state_sequence(state_sequence &&) = default;

  /* `state` overrrides */
  virtual bool contains_object_with_id(
      visible_object::objid_t id) const override;
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state>) override;
  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) override;
  virtual const visible_object_state &get_state_of_object(
      visible_object::objid_t) const override;
  virtual std::unique_ptr<state> clone() const override;

 public:
  const state_view &state_at(size_t i) const {
    return this->states_in_sequence.at(i);
  }

  /**
   * @brief Moves the contents from index 0 to index _index_ (inclusive) of this
   * sequence produce a the sequence formed by entries 0-index.
   *
   * Any `state_view` objects which were vended by the sequence via the function
   * `state_sequence::state_at()` which point to indices past _index_ are no
   * longer valid after the sequence is consumed. All other views into the
   * sequence are valid;
   *
   * @param index the last index that should be contained in the returned
   * subsequence.
   * @return the resulting subsequence. The subsequence is identical
   */
  state_sequence consume_into_subsequence(size_t index) &&;
};

}  // namespace mcmini::model
