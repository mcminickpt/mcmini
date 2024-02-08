#pragma once

#include "mcmini/model_checking/algorithm.hpp"

namespace model_checking {

/**
 * @brief A model-checking algorithm which performs verification using the
 * algorithm of Flanagan and Godefroid (2005).
 */
class classic_dpor final : public algorithm {
 public:
  void verify_using(coordinator &, const callbacks &) override;

  void verify_using(coordinator &coordinator) {
    callbacks no_callbacks;
    this->verify_using(coordinator, no_callbacks);
  }
};

}  // namespace model_checking