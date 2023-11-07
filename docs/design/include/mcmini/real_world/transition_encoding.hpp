#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/trace.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::real_world {

extern "C" {

/**
 * @brief A serialization of a _transition_ from
 *
 */
struct transition_encoding {
  volatile void *contents;
};
}

}  // namespace mcmini::real_world