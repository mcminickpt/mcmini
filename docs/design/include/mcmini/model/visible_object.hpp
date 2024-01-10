#pragma once

#include <vector>

#include "mcmini/misc/extensions/unique_ptr.hpp"
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
 *
 * A visible object is represented by its most recent state. Two visible objects
 * are considered _equal_ iff their current states are equal to one another.
 */
class visible_object final {
 private:
  std::vector<std::unique_ptr<const visible_object_state>> history;

  /**
   *@brief Construct a visible object with the given history _history_.
   */
  visible_object(
      std::vector<std::unique_ptr<const visible_object_state>> history)
      : history(std::move(history)) {}

  visible_object(const visible_object &other, size_t num_states) {
    *this = other.slice(num_states);
  }

 public:
  visible_object(visible_object &&) = default;
  visible_object &operator=(visible_object &&) = default;

  /**
   * @brief Construct a visible object with the given initial state.
   *get
   * @param initial_state the initial state of the object.
   */
  visible_object(std::unique_ptr<const visible_object_state> initial_state) {
    push_state(std::move(initial_state));
  }
  visible_object(const visible_object &other) { *this = *other.clone(); }
  visible_object &operator=(const visible_object &other) {
    return *this = *other.clone();
  }

 public:
  size_t get_num_states() const { return history.size(); }
  const visible_object_state *state_at(size_t i) const {
    return this->history.at(i).get();
  }
  const visible_object_state *get_current_state() const {
    return this->history.back().get();
  }
  void push_state(std::unique_ptr<const visible_object_state> s) {
    history.push_back(std::move(s));
  }
  /**
   * @brief Produces a visible object with the first `num_states` states of this
   * visible object.
   *
   * @param num_states the number of states that should be copied into the
   * resulting visible object.
   * @returns a visible object with identical states as this visible object for
   * the first `num_states` states.
   */
  visible_object slice(size_t num_states) const {
    auto sliced_states =
        std::vector<std::unique_ptr<const visible_object_state>>();
    sliced_states.reserve(num_states);
    for (int j = 0; j < num_states; j++) {
      sliced_states.push_back(
          mcmini::extensions::to_const_unique_ptr(history.at(j)->clone()));
    }
    return visible_object(std::move(sliced_states));
  }
  std::unique_ptr<visible_object> clone() const {
    return mcmini::extensions::make_unique<visible_object>(*this);
  }
};
}  // namespace mcmini::model