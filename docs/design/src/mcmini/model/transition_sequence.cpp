#include "mcmini/model/transitions/transition_sequence.hpp"

using namespace model;

void transition_sequence::consume_into_subsequence(uint32_t depth) {
  // For depths greater than the size of the sequence
  if (depth <= contents.size()) {
    contents.erase(contents.begin() + depth, contents.end());
  }
}

std::unique_ptr<const transition> transition_sequence::extract_at(size_t i) {
  return std::move(this->contents.at(i));
}