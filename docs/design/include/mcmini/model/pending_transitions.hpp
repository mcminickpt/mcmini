#pragma once

#include "mcmini/model/program.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/real_world/runner.hpp"

namespace mcmini::model {

/**
 * @brief A collection of "next steps" for a set of threads running in a
 * `mcmini::model::program`
 *
 * An important component of a program are the possible ways that is can evolve.
 * Evolution is described in McMini as _transitions_ -- functions of state which
 * produce a state `s'` from a given state `s`. Conceptually,
 * `pending_transitions` is simply a mapping of runner
 */
struct pending_transitions {
 private:
 public:
  const transition *get_transition_for_runner(runner::runner_id_t) const;
  std::unique_ptr<transition> displace_transition_for_thread(
      runner::runner_id_t, std::unique_ptr<transition>);
};

}  // namespace mcmini::model