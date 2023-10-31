#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/trace.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::real_world {

extern "C" {

/**
 *
 *
 */
struct transition_encoding {
  volatile void *contents;
};
}

}  // namespace mcmini::real_world