#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {
/**
 * @brief A particular snapshot in time of a program undergoing verification
 * from the perspective of the verifier.
 *
 * A `state` is simply a collection of visible objects. Each
 */
class state {
 public:
  /**
   * @brief Determines whether this state contains the object with id _id_
   *
   * @returns true if there is an object associated with id _id_ in this state,
   * and false otherwise.
   */
  virtual bool contains_object_with_id(visible_object::objid_t id) const = 0;

  /**
   * @brief Begin tracking a new visible object
   */
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state>) = 0;

  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) = 0;

  virtual const visible_object_state *get_state_of_object(
      visible_object::objid_t) const = 0;

  /**
   * @brief Creates a copy of the given state.
   *
   * All visible objects in the underlying state are copied into the new state
   * and must be independently modifiable with respect to the first state.
   */
  virtual std::unique_ptr<state> clone() const = 0;
};

class state_view : public state {
 private:
  /// @brief The sequence within which this state is a part
  ///
  /// A state is a member of state sequence
  const state_sequence *const owning_sequence;

  state_view(const state_sequence *owner) : owning_sequence(owner) {}

  friend state_sequence;

 public:
  virtual bool contains_object_with_id(
      visible_object::objid_t id) const override;
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state>) override;
  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) override;
  virtual const visible_object_state *get_state_of_object(
      visible_object::objid_t) const override;
  virtual std::unique_ptr<state> clone() const override;
};

// class detached_state : public state {
//   std::unordered_map<visible_object::objid_t,
//                      std::unique_ptr<const visible_object_state>>
//       visible_object_states_to_ids;
// };

}  // namespace mcmini::model