#pragma once

namespace mcmini::model {

/**
 * @brief A capture of the states visited in a particular branch of the state
 * space of a program undergoing verification; a trace describing how the
 * current state has come about as a sequence of transitions; and the
 * immediately visible "next steps" that can be taken.
 *
 * A _mcmini::model::program_ is a container that comprises of the following
 * three components:
 *
 * - a sequence T of transitions that have occurred called a _trace_.
 * - a model of the state of the program undergoing verification after each
 * point `t` in trace `T`.
 * - a mapping between the individual execution units of the program ("threads")
 * and the next step that thread will take once executed.
 *
 *
 *
 *
 *
 */
class program {
  // state_space_branch??
  // Snapshot
  // trace
  // thred state snapshot

  // give me a new program N steps back ->
};
//

};  // namespace mcmini::model