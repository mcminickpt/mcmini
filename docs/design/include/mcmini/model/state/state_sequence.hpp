#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/append-only.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::model {

/**
 * @brief A sequence of states.
 *
 * A _state sequence_ describes how a `mcnini::model:;program`'s state has
 * changed over time as its execution units have executed and taken action.
 *
 * A _state_ is an unordered container of visible objects. Each visible object
 * is in turn composed of a sequence of a finite number of states.
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
  /**
   * @brief An element of a `mcmini::model::state_sequence`
   *
   * The state_view and state_sequence are tightly
   * intertwined. We allow them to work in tandem with one
   * another as an implementation detail
   */
  class element : public state {
   private:
    /// @brief A collection of references to states in the sequence
    /// _owning_sequence_ to which this element belongs.
    ///
    /// Each state in the view
    std::vector<const visible_object_state *> visible_object_states;

    element(const state_sequence &owner);
    friend state_sequence;

   public:
    virtual bool contains_object_with_id(objid_t id) const override;
    virtual const visible_object_state *get_state_of_object(
        objid_t id) const override;
    virtual std::unique_ptr<mutable_state> mutable_clone() const override;
  };

  /**
   * @brief A state which has a base
   */
  class diff_state;

  /**
   * @brief Consume the differences contained in the state given which produces
   * the diff.
   */
  void consume_diff(diff_state &&);

  // INVARIANT: As new states are added to the visible objects in the
  // mapping, new state views are also added with the appropriate object states
  // replaced.

  // INVARIANT: Objects must only be added to the collection and are never
  // removed.
  mcmini::append_only<visible_object> visible_objects;
  mcmini::append_only<element> states_in_sequence;

 public:
  state_sequence(const state &);
  state_sequence(const state &&);
  state_sequence(state_sequence &) = delete;
  state_sequence(state_sequence &&) = default;
  state_sequence(std::vector<visible_object> &&);
  state_sequence &operator=(const state_sequence &&) = delete;
  state_sequence &operator=(const state_sequence &) = delete;

  /* `state` overrrides */
  virtual bool contains_object_with_id(state::objid_t id) const override;
  virtual const visible_object_state *get_state_of_object(
      objid_t id) const override;
  virtual std::unique_ptr<mutable_state> mutable_clone() const override;

  /* Applying transitions */

  /**
   * @brief Applies the given transition to the final state of the sequence if
   * it is enabled there and pushes the adds the transformed state to the
   * sequence.
   *
   * @return whether the transition was enabled at the final state in the
   * sequence. If the transition was disabled there, a new state will _not_ be
   * added to the sequence and a `disabled` status will be returned. OTherwise,
   * the transition is _defined_ at the final state and a new state `s'` is
   * added to the end of the sequence.
   */
  transition::status follow(const transition &t);

  const state &state_at(size_t i) const {
    return this->states_in_sequence.at(i);
  }

  /**
   * @brief Moves the contents from index 0 to index _index_ (inclusive) of this
   * sequence produce a the sequence formed by entries 0-index.
   *
   * Any `state` references which were vended by the sequence via the function
   * `state_sequence::state_at()` which point to indices past _index_ are no
   * longer valid after the sequence is consumed. All other views into the
   * sequence remain valid.
   *
   * @param index the last index that should be contained in the returned
   * subsequence.
   * @return the resulting subsequence. The subsequence is identical to this
   * sequence up to index `index`
   */
  state_sequence consume_into_subsequence(size_t index) &&;
};

}  // namespace mcmini::model
