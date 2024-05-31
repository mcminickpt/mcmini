#pragma once

#include <vector>

#include "mcmini/misc/extensions/unique_ptr.hpp"
#include "mcmini/model/visible_object_state.hpp"

namespace model {

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
 * object appeared during the execution of a `model::program`. All
 * objects own the states that represent them.
 *
 * A visible object is represented by its most recent state. Two visible objects
 * are considered _equal_ iff their current states are equal to one another.
 */
class visible_object final {
 private:
  std::vector<const visible_object_state *> history;

  /**
   * @brief Construct a visible object with the given history _history_.
   */
  visible_object(std::vector<const visible_object_state *> history)
      : history(std::move(history)) {}

  visible_object(const visible_object &other, size_t num_states) {
    *this = other.copy_slice(num_states);
  }

 public:
  visible_object() = default;
  visible_object(visible_object &&) = default;
  visible_object &operator=(visible_object &&) = default;
  visible_object(const visible_object_state *initial_state) {
    push_state(initial_state);
  }
  visible_object(std::unique_ptr<const visible_object_state> initial_state)
      : visible_object(initial_state.release()) {}
  visible_object(const visible_object &other) {
    *this = other.copy_slice(other.get_num_states());
  }
  visible_object &operator=(const visible_object &other) {
    this->history.reserve(other.get_num_states());
    for (size_t j = 0; j < other.get_num_states(); j++)
      this->history.push_back(other.history.at(j)->clone().release());
    return *this;
  }
  ~visible_object();

 public:
  size_t get_num_states() const { return history.size(); }
  size_t get_last_state_index() const { return history.size() - 1; }
  const visible_object_state *state_at(size_t i) const {
    return this->history.at(i);
  }
  const visible_object_state *get_current_state() const {
    return this->history.back();
  }
  void push_state(const visible_object_state *s) { history.push_back(s); }
  /**
   * @brief Produces a visible object with the first `num_states` states of this
   * visible object.
   *
   * The object will be left with the states `[0, index]` (inclusive).
   */
  void slice(size_t index);

  /**
   * @brief Produces a visible object with the first `num_states` states of this
   * visible object.
   *
   * @param num_states the number of states that should be copied into the
   * resulting visible object.
   * @returns a visible object with identical states as this visible object for
   * the first `num_states` states.
   */
  visible_object copy_slice(size_t num_states) const {
    std::vector<const visible_object_state *> sliced_states;
    sliced_states.reserve(num_states);
    for (size_t j = 0; j < num_states; j++) {
      sliced_states.push_back(history.at(j)->clone().release());
    }
    return visible_object(std::move(sliced_states));
  }

  /// @brief Extracts the current state from this object.
  /// @return a pointer to the current state of this object, or `nullptr` is the
  /// object contains no states. After calling this method, the visible object
  /// should be considered destroyed
  std::unique_ptr<const visible_object_state> consume_into_current_state() {
    if (history.empty()) {
      return nullptr;
    }
    auto result = std::unique_ptr<const visible_object_state>(history.back());
    history.pop_back();
    return result;
  }

  std::unique_ptr<visible_object> clone() const {
    return extensions::make_unique<visible_object>(*this);
  }
};
}  // namespace model
