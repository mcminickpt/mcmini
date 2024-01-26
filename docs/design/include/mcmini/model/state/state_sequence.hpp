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
 * A _state sequence_ is a sequence of states with the following properties:
 *
 * 1. for each state `s_i`, there exists some transition `t_i` such that `t_i`
 * is defined at `s_i` (`t_i` is enabled there) and `s_{i+1} = t_i(s_i)`. That
 * is, given any pair of states `s_i` and `s_j` in the sequence (i <= j), there
 * exists a sequence of transitions `t_i, ..., t_j` such that `s_j =
 * t_j(t_{j-1}(...(t_i(s_i))...))`
 */
class state_sequence : public state {
 private:
  /**
   * @brief An element of a `mcmini::model::state_sequence`
   *
   * The `element` and `state_sequence` are tightly intertwined. We allow them
   * to work in tandem with one another as an implementation detail to permit
   * "views" of the objects the state sequence maintains as new states are added
   * to the sequence via transitions
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
   * @brief A state which maintains changes to an underlying base state.
   *
   * A `diff_state` manages the changes in state of objects which exist in a
   * given state, as well as any newly-created objects
   */
  class diff_state;

  /**
   * @brief Consume the differences contained in the `diff_state`
   */
  void consume_diff(const diff_state &);

  // INVARIANT: As new states are added to the visible objects in the
  // mapping, new state views are also added with the appropriate object states
  // replaced.
  mcmini::append_only<visible_object> visible_objects;
  mcmini::append_only<element> states_in_sequence;

 public:
  state_sequence() = default;
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
   * added to the sequence and a `disabled` status will be returned. Otherwise,
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
   * sequence up to index `index`. Any objects which didn't exist prior to state
   * `s_index` will not exist in the resulting sequence
   */
  state_sequence consume_into_subsequence(size_t index) &&;
};

}  // namespace mcmini::model
