#pragma once

#include <vector>

#include "mcmini/model/visible_object_state.hpp"

namespace mcmini::model {

/**
 * @brief A placeholder which represents a snapshot of something that multiple
 * threads interact with to communicate with each other in a program.
 *
 * A _visible object_, from the perspective of a model checker, are those
 * portions of a program which are semantically interesting with respect to
 * verification. Threads in a program interact with one another by communicating
 * with one another through operations (known as _visible operations_) which act
 * upon visible objects to transmit information from thread to thread.
 *
 * A visible object is comprised of a collection of states describing how that
 * object appeared during the execution of a `mcmini::model::program`. All
 * objects
 */
class visible_object {
 private:
  std::vector<std::unique_ptr<const visible_object_state>> history;

 public:
  using objid_t = uint32_t;

  /**
   * @brief Construct a visible object with the given initial state
   *
   * @param initial_state the initial state of the object
   */
  visible_object(std::unique_ptr<visible_object_state> initial_state);

  size_t get_num_states() const { return history.size(); }
  const visible_object_state *state_at(size_t i) const {
    return this->history.at(i).get();
  }

  // auto begin() const { return this->history.begin(); }
  // auto end() const { return this->history.end(); }
};
}  // namespace mcmini::model