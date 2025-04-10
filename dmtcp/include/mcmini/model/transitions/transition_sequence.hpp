#pragma once

#include <stdint.h>

#include <vector>

#include "mcmini/model/transition.hpp"

namespace model {

/**
 * @brief A sequence of transitions.
 *
 * A _transition sequence_ describes a sequence of consecutive executions. The
 * order that transitions appear in the sequence corresponds to how those
 * transitions were executed one after another in a program undergoing
 * verification. However, a transition sequence is a sequence and _only_ a
 * sequence of transitions; it is not guaranteed to represent a possible
 * execution path of a (or any) program; that is, there are no constraints on
 * the enabled-ness of transitions in the sequence.
 */
class transition_sequence final {
 private:
  std::vector<const transition*> contents;

 public:
  using index = size_t;
  transition_sequence() = default;
  ~transition_sequence();

  auto begin() -> decltype(contents.begin()) { return contents.begin(); }
  auto end() -> decltype(contents.end()) { return contents.end(); }
  auto begin() const -> decltype(contents.begin()) { return contents.begin(); }
  auto end() const -> decltype(contents.end()) { return contents.end(); }

  bool empty() const { return contents.empty(); }
  size_t count() const { return contents.size(); }
  const transition* at(size_t i) const { return contents.at(i); }
  const transition* back() const { return contents.back(); }
  std::unique_ptr<const transition> extract_at(size_t i);
  void push(const transition* t) { contents.push_back(t); }
  void consume_into_subsequence(uint32_t depth);
};

}  // namespace model
