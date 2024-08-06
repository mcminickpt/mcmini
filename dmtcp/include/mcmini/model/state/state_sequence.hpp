#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/append-only.hpp"
#include "mcmini/model/state/detached_state.hpp"
#include "mcmini/model/state/diff_state.hpp"
#include "mcmini/model/transition.hpp"

namespace model {

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
 *
 * A `state_sequence` is itself a `model::mutable_state` that is conceptually
 * represented by the final state in the sequence. A state sequence is never
 * empty,
 */
class state_sequence final : public mutable_state {
 private:
  class element;
  injective_function<runner_id_t, state::objid_t> runner_to_obj_map;
  append_only<model::visible_object> visible_objects;

  /// @brief Inserts an instance of `element` in the `states_in_sequence`
  void push_state_snapshot();

  // INVARIANT: As new states are added to the visible objects in the
  // mapping `visible_objects`, new state views are also added with the
  // appropriate object states replaced for the _last element_ of
  // `states_in_sequence`. When new objects are added to `visible_objects`, a
  // corresponding object state is added to the _last_ element in the sequence.
  append_only<element *> states_in_sequence;

  /// @brief Retrieves the state which represents that sequence as a whole.
  element &get_representative_state() const {
    return *this->states_in_sequence.back();
  }

 public:
  state_sequence();
  ~state_sequence();
  state_sequence(const state &);
  state_sequence(state_sequence &&);
  state_sequence(std::vector<visible_object> &&);
  state_sequence(append_only<visible_object> &&);
  state_sequence &operator=(const state_sequence &&) = delete;
  state_sequence &operator=(const state_sequence &) = delete;

  size_t count() const override;
  size_t runner_count() const override;
  size_t get_num_states_in_sequence() const;

  objid_t get_objid_for_runner(runner_id_t id) const override;
  runner_id_t get_runner_id_for_obj(objid_t id) const override;
  bool is_runner(objid_t id) const override;
  bool contains_object_with_id(state::objid_t id) const override;
  bool contains_runner_with_id(runner_id_t id) const override;
  const visible_object_state *get_state_of_object(objid_t id) const override;
  const runner_state *get_state_of_runner(runner_id_t id) const override;
  objid_t add_object(const visible_object_state *) override;
  runner_id_t add_runner(const runner_state *) override;
  void add_state_for_obj(objid_t id, const visible_object_state *) override;
  void add_state_for_runner(runner_id_t id, const runner_state *) override;
  std::unique_ptr<mutable_state> mutable_clone() const override;

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

  const state &front() const;
  const state &back() const;
  const state &state_at(size_t i) const;

  /**
   * @brief Moves the contents from index 0 to index _index_ (inclusive) of this
   * sequence produce a the sequence formed by entries 0-index.
   *
   * Any `state` references which were vended by the sequence via the function
   * `state_sequence::state_at()` which point to indices past _index_ are no
   * longer valid after the sequence is consumed. All other views into the
   * sequence remain valid.
   *
   * @param num_states the number of states that should be left in the sequence
   * after consumption. If `num_states` is large than the number of states in
   * this sequence, the method has no effect.
   */
  void consume_into_subsequence(size_t num_states);
};

}  // namespace model
