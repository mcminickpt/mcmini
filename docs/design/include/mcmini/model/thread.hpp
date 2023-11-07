#pragma once

#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {

/**
 *
 *
 */
class thread final : public visible_object {
 public:
  using tid_t = uint32_t;
};

}  // namespace mcmini::model