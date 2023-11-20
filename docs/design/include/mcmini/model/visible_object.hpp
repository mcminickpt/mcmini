#pragma once

#include <vector>

#include "mcmini/model/visible_object_state.hpp"

namespace mcmini::model {

/**
 * @brief A placeholder which represents a snapshot of an object with which
 * multiple threads interact to communicate in a program.
 *
 * A _visible object_, from the perspective of a model checker, are those
 * portions of a program which are semantically interesting with respect to
 * verification. Threads in a program interact with one another by communicating
 * with one another through operations (known as _visible operations_) that act
 * upon visible objects to transmit information from one thread to another.
 *
 * A visible object is comprised of a collection of states describing how that
 * object appeared during the execution of a `mcmini::model::program`. All
 * objects own the states that represent them.
 */
class visible_object {
 private:
  std::vector<std::unique_ptr<const visible_object_state>> history;

  /**
   *@brief Construct a visible object with the given history _history_.
   */
  visible_object(
      std::vector<std::unique_ptr<const visible_object_state>> history)
      : history(std::move(history)) {}

 public:
  using objid_t = uint32_t;

  /**
   * @brief Construct a visible object with the given initial state.
   *get
   * @param initial_state the initial state of the object.
   */
  visible_object(std::unique_ptr<const visible_object_state> initial_state) {
    push_state(std::move(initial_state));
  }

  /* State Management */
  size_t get_num_states() const { return history.size(); }
  template <typename T>
  const T *state_at(size_t i) const {
    return static_cast<T *>(this->history.at(i).get());
  }
  template <typename T>
  const T *get_curent_state() const {
    return static_cast<T *>(this->history.back().get());
  }
  void push_state(std::unique_ptr<const visible_object_state> s) {
    history.push_back(std::move(s));
  }

  /* Slices */
  visible_object slice(size_t i) const {
    auto sliced_states =
        std::vector<std::unique_ptr<const visible_object_state>>();
    for (int j = 0; j < i; j++) {
      std::unique_ptr<const visible_object_state> const_copy(
          static_cast<const visible_object_state *>(
              history.at(i)->clone().release()));
      sliced_states.push_back(std::move(const_copy));
    }
    return visible_object(std::move(sliced_states));
  }
};
}  // namespace mcmini::model