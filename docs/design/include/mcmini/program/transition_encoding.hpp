#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/trace.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::verification {

extern "C" {

/**
 *
 *
 */
struct transition_encoding {
  volatile void *contents;
};
}

static_assert(decltype(transition_encoding) == decltype(transition_encoding));

}  // namespace mcmini::verification