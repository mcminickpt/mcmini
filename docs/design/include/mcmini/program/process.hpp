#pragma once

#include <memory>

#include "mcmini/model/thread.hpp"
#include "mcmini/model/trace.hpp"
#include "mcmini/model/transition.hpp"

namespace mcmini::verification {

/**
 *
 *
 */
struct process {
 public:
  process();

  const trace &get_trace() const;

  /**
   *
   */
  void execute_thread_in_model(mcmini::model::thread::id);
};

}  // namespace mcmini::verification