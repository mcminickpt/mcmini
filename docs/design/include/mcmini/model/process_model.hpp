#pragma once

#include "mcmini/model/state.hpp"

namespace mcmini::model {

/**
 * A
 *
 */
class process_model {
 public:
  const mcmini::model::state &get_current_state() const;
};

}  // namespace mcmini::model
