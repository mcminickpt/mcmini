#include "mcmini/model/transitions/transition_sequence.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "mcmini/misc/extensions/memory.hpp"
#include "mcmini/model/transition.hpp"

using namespace model;

void transition_sequence::consume_into_subsequence(uint32_t depth) {
  // For depths greater than the size of the sequence, this method has no
  // effect.
  if (depth <= contents.size()) {
    extensions::delete_all(contents.begin() + depth, contents.end());
    contents.erase(contents.begin() + depth, contents.end());
  }
}

std::unique_ptr<const transition> transition_sequence::extract_at(size_t i) {
  auto result = std::unique_ptr<const transition>(this->contents.at(i));
  this->contents.at(i) = nullptr;
  return result;
}

transition_sequence::~transition_sequence() {
  extensions::delete_all(contents.begin(), contents.end());
}
