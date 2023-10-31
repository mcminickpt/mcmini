#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::real_world {

/**
 *
 *
 */
struct process {
 public:
  process();

  /**
   *
   */
  void execute_thread_in_model(mcmini::model::thread::id);
};

}  // namespace mcmini::real_world