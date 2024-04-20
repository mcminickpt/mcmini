#include "mcmini/model/program.hpp"

using namespace model;

program::program(const state &initial_state,
                 pending_transitions &&initial_first_steps)
    : next_steps(std::move(initial_first_steps)), state_seq(initial_state) {}