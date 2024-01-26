#include "mcmini/model/program.hpp"

using namespace mcmini::model;

program::program(state &&initial_state,
                 pending_transitions &&initial_first_steps)
    : next_steps(std::move(initial_first_steps)),
      state_seq(std::move(initial_state)) {}