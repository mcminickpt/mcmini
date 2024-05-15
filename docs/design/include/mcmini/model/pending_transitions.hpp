#pragma once

#include <map>
#include <memory>

#include "mcmini/model/defines.hpp"
#include "mcmini/model/transition.hpp"

namespace model {

/**
 * @brief A collection of "next steps" for a set of threads running in a
 * `model::program`
 *
 * An important component of a program are the possible ways that is can evolve.
 * Evolution is described in McMini as _transitions_ -- functions of state which
 * produce a state `s'` from a given state `s`. Conceptually,
 * `pending_transitions` is a mapping of runner ids to transitions.
 */
struct pending_transitions final {
 private:
  using runner_id_t = ::runner_id_t;
  std::map<runner_id_t, std::unique_ptr<const transition>> _contents;

 public:
  auto begin() -> decltype(_contents.begin()) { return _contents.begin(); }
  auto end() -> decltype(_contents.end()) { return _contents.end(); }
  auto begin() const -> decltype(_contents.cbegin()) {
    return _contents.cbegin();
  }
  auto end() const -> decltype(_contents.cend()) { return _contents.cend(); }
  auto cbegin() -> decltype(_contents.cbegin()) const {
    return _contents.cbegin();
  }
  auto cend() -> decltype(_contents.cend()) const { return _contents.cend(); }
  size_t size() const { return this->_contents.size(); }
  /**
   * @brief Returns the transition mapped to id `id`, or `nullptr` if no such
   * runner has been mapped to an id.
   *
   * @param id the id of the runner whose transition should be retrieved.
   */
  const transition *get_transition_for_runner(runner_id_t id) const {
    if (_contents.count(id) > 0) {
      return _contents.at(id).get();
    }
    return nullptr;
  }

  std::unique_ptr<const transition> displace_transition_for(
      runner_id_t id, std::unique_ptr<const transition> new_transition) {
    if (id != new_transition->get_executor()) {
      throw std::runtime_error(
          "Attempting to insert a transition executed by a different runner (" +
          std::to_string(id) +
          " != " + std::to_string(new_transition->get_executor()) + ")");
    }
    auto old_transition = std::move(_contents[id]);
    _contents[id] = std::move(new_transition);
    return old_transition;
  }
};

}  // namespace model