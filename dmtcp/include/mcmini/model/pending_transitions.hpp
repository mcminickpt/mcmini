#pragma once

#include <map>
#include <memory>

#include "mcmini/misc/extensions/unique_ptr.hpp"
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
  std::map<runner_id_t, const transition *> _contents;

 public:
  pending_transitions() = default;
  pending_transitions(pending_transitions &&) = default;
  pending_transitions(const pending_transitions &) = delete;
  pending_transitions &operator=(pending_transitions &&) = default;
  pending_transitions &operator=(const pending_transitions &) = delete;
  ~pending_transitions() {
    for (const auto &p : _contents) delete p.second;
  }
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
      return _contents.at(id);
    }
    return nullptr;
  }

  std::unique_ptr<const transition> replace_managed(
      const transition *new_transition) noexcept {
    return std::unique_ptr<const transition>(replace_unowned(new_transition));
  }

  /// @brief Replace the pending operation for `new_transition->get_executor()`
  /// @param new_transition the transition that will be used to replace any
  /// current transition in the struct (ownership is acquired)
  /// @return a pointer to the old transition. This pointer must be freed using
  /// `delete`.
  const transition *replace_unowned(const transition *new_transition) noexcept {
    runner_id_t id = new_transition->get_executor();
    const transition *old_transition = _contents[id];
    _contents[id] = new_transition;
    return old_transition;
  }

  void set_transition(const transition *new_transition) noexcept {
    delete replace_unowned(new_transition);
  }
};

}  // namespace model
