#pragma once

#include <functional>

#include "mcmini/model/state.hpp"

namespace mcmini::model {

class state_view : public state {
 private:
  /// @brief A collection of references to states in the sequence
  /// _owning_sequence_ for which this state view points into.
  ///
  /// Each state in the view.
  std::vector<const visible_object_state *> visible_object_states;

  /// @brief The sequence within which this state is a part
  ///
  /// A state is a member of state sequence
  state_sequence &owning_sequence;
  state_view(state_sequence &owner);
  friend state_sequence;

 public:
  virtual bool contains_object_with_id(
      visible_object::objid_t id) const override;
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state>) override;
  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) override;
  virtual const visible_object_state &get_state_of_object(
      visible_object::objid_t) const override;
  virtual std::unique_ptr<state> clone() const override;
};

}  // namespace mcmini::model
