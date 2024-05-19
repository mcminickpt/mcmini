#include "mcmini/model/program.hpp"

#include <algorithm>

using namespace model;

program::program(const state &initial_state,
                 pending_transitions &&initial_first_steps)
    : next_steps(std::move(initial_first_steps)), state_seq(initial_state) {}

std::unordered_set<state::runner_id_t> program::get_enabled_runners() const {
  std::unordered_set<runner_id_t> enabled_runners;
  for (const auto &runner_and_t : this->next_steps) {
    if (runner_and_t.second->is_enabled_in(state_seq)) {
      enabled_runners.insert(runner_and_t.first);
    }
  }
  return enabled_runners;
}

void program::restore_model_at_depth(uint32_t n) {
  /*
   * Fill in the set of next transitions by
   * following the trace from the top to `n` since the trace implicitly holds
   * what each thread *was* doing next at each point in time.
   *
   * We can simply keep track of the _smallest_ index in the trace *greater than
   * `n`* for each thread. This must be the most recent transition
   * that that was pending for the thread at depth `n`
   *
   * For threads that didn't run after depth _depth_, the latest transition in
   * `next_steps` is already correct and we don't need to do anything.
   *
   * @note It's possible that some of these threads will be
   * in the embryo state at depth `n`; those threads should consequently be
   * executing the `thread_start()` transition, but this will be in the trace
   */
  std::unordered_map<runner_id_t, uint32_t> runner_to_index_from_top;
  for (int32_t i = (trace.count() - 1); i >= (int32_t)(n); i--)
    runner_to_index_from_top[trace.at(i)->get_executor()] = i;

  for (const std::pair<runner_id_t, uint32_t> &e : runner_to_index_from_top)
    next_steps.displace_transition_for(e.first, trace.extract_at(e.second));

  state_seq.consume_into_subsequence(n);
  trace.consume_into_subsequence(n);
}

void program::model_execution_of(runner_id_t p,
                                 std::unique_ptr<transition> new_transition) {
  if (p != new_transition->get_executor()) {
    throw std::runtime_error(
        "The next incoming transition replacing `next_s_p` in the model must "
        "be run by the same runner (" +
        std::to_string(p) +
        " != " + std::to_string(new_transition->get_executor()) + ")");
  }

  const transition *next_s_p = next_steps.get_transition_for_runner(p);
  if (!next_s_p) {
    // This is the first time we've seen `p`; treat `new_transition` as the
    // first `next_s_p`.
    throw std::runtime_error(
        "Attempting to execute a runner whose next transition is unknown to "
        "the model. If this runner was dynamically created in the model (e.g. "
        "in the callback routine for handling `pthread_create()`), ensure that "
        "the ");
  }

  transition::status status = this->state_seq.follow(*next_s_p);
  if (status == transition::status::disabled) {
    throw std::runtime_error(
        "Attempted to model the execution of a disabled transition(" +
        next_s_p->debug_string() + ")");
  }
  trace.push(next_steps.displace_transition_for(p, std::move(new_transition)));
}

state::objid_t program::discover_object(
    std::unique_ptr<visible_object_state> initial_state) {
  return this->state_seq.add_object(std::move(initial_state));
}

state::runner_id_t program::discover_runner(
    std::unique_ptr<visible_object_state> initial_state,
    runner_generation_function f) {
  state::runner_id_t id = this->state_seq.add_runner(std::move(initial_state));
  this->next_steps.displace_transition_for(id, f(id));
  return id;
}

bool program::is_in_deadlock() const {
  for (const auto &t : this->get_pending_transitions()) {
    if (t.second->is_enabled_in(this->state_seq)) return false;
  }
  return true;
}