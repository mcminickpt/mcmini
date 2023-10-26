#pragma once

#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {

/**
 *
 *
 */
class thread final : public visible_object {
 public:
  /**
   *
   */
  using id = uint32_t;

  visible_object::id get_identifier_in_modelchecking_model() const;

  std::unique_ptr<visible_object> clone() {
    return std::unique_ptr<thread>(new thread());
  }
};

}  // namespace mcmini::model