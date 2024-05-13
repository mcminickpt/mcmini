#include "mcmini/model_checking/algorithms/classic_dpor.hpp"

#include <cassert>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "mcmini/defines.h"
#include "mcmini/model/program.hpp"
#include "mcmini/signal.hpp"

using namespace model;
using namespace model_checking;

void classic_dpor::verify_using(coordinator &coordinator,
                                const callbacks &callbacks) {
  // The code below is an implementation of the model-checking algorithm of
  // Flanagan and Godefroid from 2015.

  // 1. Data structure set up
  std::unordered_map<runner_id_t, runner_item> per_runner_clocks;

  /// @invariant: The number of items in the DPOR-specific stack is the same
  /// size as the number of transitions in the current trace plus one.
  ///
  /// The initial entry into the stack represents the information DPOR tracks
  /// for state `s_0`.
  std::vector<stack_item> dpor_stack;
  dpor_stack.emplace_back(
      clock_vector(),
      coordinator.get_current_program_model().get_enabled_runners());

  while (!dpor_stack.empty()) {
    // 2. Exploration phase
    while (dpor_stack.back().has_enabled_runners()) {
      if (dpor_stack.size() >= MAX_TOTAL_TRANSITIONS_IN_PROGRAM) {
        throw std::runtime_error(
            "*** Execution Limit Reached! ***\n\n"
            "McMini ran a trace with" +
            std::to_string(dpor_stack.size()) +
            " transitions which is\n"
            "the most McMini can currently handle in any one trace. Try\n"
            "running mcmini with the \"--max-depth-per-thread\" flag\n"
            "to limit how far into a trace McMini can go\n");
      }

      {  // 2a. Execute the runner in the model and in the real world
        // For deterministic results, always choose the smallest runner
        const auto &enabled_runners = dpor_stack.back().get_enabled_runners();
        runner_id_t venturer = *enabled_runners.begin();
        for (const runner_id_t rid : enabled_runners)
          venturer = std::min(rid, venturer);
        coordinator.execute_runner(venturer);
      }

      {  // 2b. Update DPOR data structures (per-thread data, clock vectors,
         // backtrack sets)
        this->grow_stack_after_running(coordinator, per_runner_clocks,
                                       dpor_stack);
        this->dynamically_update_backtrack_sets(coordinator, dpor_stack);
      }
    }

    // 3. Backtrack phase
    do {
      // Locate a spot that contains backtrack threads
      if (dpor_stack.back().backtrack_set_empty()) {
        dpor_stack.pop_back();
      } else {
        break;
      }
    } while (!dpor_stack.empty());
  }
}

clock_vector classic_dpor::accumulate_max_clock_vector_against(
    const model::transition &t, const std::vector<stack_item> &stack) const {
  // The last state in the stack does NOT have an out transition, hence the
  // `nullptr` check. Note that `s_i.get_out_transition()` refers to `S_i`
  // (case-sensitive) in the paper, viz. the transition between states `s_i` and
  // `s_{i+1}`.
  clock_vector result;
  for (const stack_item &s_i : stack) {
    if (s_i.get_out_transition() != nullptr &&
        this->are_dependent(*s_i.get_out_transition(), t)) {
      result = clock_vector::max(result, s_i.get_clock_vector());
    }
  }
  return result;
}

void classic_dpor::grow_stack_after_running(
    const coordinator &coordinator,
    std::unordered_map<runner_id_t, runner_item> &per_runner_clocks,
    std::vector<model_checking::stack_item> &stack) {
  // In this method, the following invariants are assumed to hold:
  //
  // 1. `n` := `stack.size()`.
  // 2. `t_n` is the `n`th transition executed in the model of `coordinator`.
  // This transition *has already executed in the model.* THIS IS VERY
  // IMPORTANT as the DPOR information tracking below relies on this heavily.
  //
  // After this method is executed, the stack will have size `n + 1`. Each entry
  // will correspond to the information DPOR cares about for each state in the
  // `coordinator`'s state sequence.
  assert(coordinator.get_depth_into_program() == stack.size() + 1);
  const model::transition *t_n =
      coordinator.get_current_program_model().get_trace().back();

  // NOTE: `cv` corresponds to line 14.3 of figure 4 in the DPOR paper.
  clock_vector cv = accumulate_max_clock_vector_against(*t_n, stack);

  // NOTE: The assignment corresponds to line 14.4 of figure 4. Here `S'`
  // represents the transition sequence _after_ `t_n` has executed. Since the
  // `coordinator` already contains this transition (recall the invariants in
  // the comment at the start of this function), `S' ==
  // `coordinator.get_current_program_model().get_trace()` and thus `|S'| ==
  // corodinator.get_depth_into_program().
  cv[t_n->get_executor()] = coordinator.get_depth_into_program();

  // NOTE: This line corresponds to line 14.5 of figure 4. Here, C' is
  // conceptually captured through the DPOR stack and the per-thread DPOR data.
  // The former contains the per-state clock vectors while the latter the
  // per-thread clock vectors (among other data).
  stack_item &s_n = stack.back();
  per_runner_clocks[t_n->get_executor()].set_clock_vector(cv);
  stack.emplace_back(
      cv, t_n, coordinator.get_current_program_model().get_enabled_runners());
  stack_item &s_n_plus_1 = stack.back();

  // INVARIANT: For each thread `p`, if such a thread is contained
  // in the sleep set of `s_n`, then `next(s_n, p)` MUST be the transition
  // that would be contained in that sleep set.
  for (const runner_id_t &rid : s_n.get_sleep_set()) {
    const model::transition *rid_next =
        coordinator.get_current_program_model().get_pending_transition_for(rid);
    if (this->are_independent(*rid_next, *t_n))
      s_n_plus_1.insert_into_sleep_set(rid);
  }

  // `t_n` is inserted into the sleep set AFTER execution. This is how sleep
  // sets work (see papers etc.)
  s_n.insert_into_sleep_set(t_n->get_executor());
  s_n.mark_searched(t_n->get_executor());
}

void classic_dpor::dynamically_update_backtrack_sets(
    const coordinator &coordinator, std::vector<stack_item> &stack) {
  /*
   * Updating the backtrack sets is accomplished as follows
   * (under the given assumptions)
   *
   * ASSUMPTIONS
   *
   * 1. The state reflects last(S) for the transition stack
   *
   * 2. The thread that ran last is at the top of the transition
   * stack (this should always be true)
   *
   * 3. The next transition for the thread that ran the most
   * recent transition in the transition stack (the transition at the
   * top of the stack) has been properly updated to reflect what that
   * thread will do next
   *
   * WLOG, assume there are `n` transitions in the transition stack
   * and `k` threads that are known to exist at the time of updating
   * the backtrack sets. Note this implies that there are `n+1` items
   * in the state stack (since there is always the initial state + 1
   * for every subsequent transition thereafter)
   *
   * Let
   *  S_i = ith backtracking state item
   *  T_i = ith transition
   *  N_p = the next transition for thread p (next(s, p))
   *
   * ALGORITHM:
   *
   * 1. First, get a reference to the transition at the top
   * of the transition stack (i.e. the most recent transition)
   * as well as the thread that ran that transition. WLOG suppose that
   * thread has a thread id `i`.
   *
   * This transition will be used to test against the transitions
   * queued as running "next" for all of the **other** threads
   * that exist
   *
   *  2. Test whether a backtrack point is needed at state
   *  S_n for the other threads by comparing N_p, for all p != i.
   *
   *  3. Get a reference to N_i and traverse the transition stack
   *  to determine if a backtrack point is needed anywhere for
   *  thread `i`
   */
  const size_t num_threads =
      coordinator.get_current_program_model().get_num_runners();

  std::unordered_set<tid_t> thread_ids;
  thread_ids.reserve(num_threads);
  for (tid_t i = 0; i < num_threads; i++) thread_ids.insert(i);

  const ssize_t tStackTop = (ssize_t)(stack.size()) - 1;
  const runner_id_t last_runner_to_execute =
      coordinator.get_current_program_model()
          .get_trace()
          .back()
          ->get_executor();
  thread_ids.erase(last_runner_to_execute);

  // O(# threads)
  {
    const model::transition &S_n =
        *coordinator.get_current_program_model().get_trace().back();

    for (runner_id_t rid = 0; rid < num_threads; rid++) {
      const model::transition &nextSP = *coordinator.get_current_program_model()
                                             .get_pending_transitions()
                                             .get_transition_for_runner(rid);
      dynamically_update_backtrack_sets_at_index(S_n, nextSP, stack.back(),
                                                 tStackTop, rid);
    }
  }

  // O(transition stack size)
  {
    const model::transition &next_s_p_for_latest_runner =
        *coordinator.get_current_program_model()
             .get_pending_transitions()
             .get_transition_for_runner(last_runner_to_execute);

    // It only remains to add backtrack points at the necessary
    // points for thread `last_runner_to_execute`. We start at one step elow the
    // top since we know that transition to not be co-enabled (since it was, by
    // assumption, run by `last_runner_to_execute`)
    for (int i = tStackTop - 1; i >= 0; i--) {
      const model::transition &S_i =
          *coordinator.get_current_program_model().get_trace().at(i);
      const bool shouldStop = dynamically_update_backtrack_sets_at_index(
          S_i, next_s_p_for_latest_runner, stack.at(i), i,
          last_runner_to_execute);
      /*
       * Stop when we find the _first_ such i; this
       * will be the maxmimum `i` since we're searching
       * backwards
       */
      if (shouldStop) break;
    }
  }
}

// bool MCStack::happensBefore(int i, int j) const {
//   MC_ASSERT(i >= 0 && j >= 0);
//   const tid_t tid = getThreadRunningTransitionAtIndex(i);
//   const MCClockVector cv = clockVectorForTransitionAtIndex(j);
//   return i <= cv.valueForThread(tid).value_or(0);
// }

// bool MCStack::happensBeforeThread(int i, tid_t p) const {
//   const tid_t tid = getThreadRunningTransitionAtIndex(i);
//   const MCClockVector cv = getThreadDataForThread(p).getClockVector();
//   return i <= cv.valueForThread(tid).value_or(0);
// }

// bool MCStack::threadsRaceAfterDepth(int depth, tid_t q, tid_t p) const {
//   // We want to search the entire transition stack in this case
//   const auto transitionStackHeight = this->getTransitionStackSize();
//   for (int j = depth + 1; j < transitionStackHeight; j++) {
//     if (q == this->getThreadRunningTransitionAtIndex(j) &&
//         this->happensBeforeThread(j, p))
//       return true;
//   }
//   return false;
// }

bool classic_dpor::dynamically_update_backtrack_sets_at_index(
    const model::transition &S_i, const model::transition &nextSP,
    stack_item &preSi, int i, int p) {
  // TODO: add in co-enabled conditions
  const bool has_reversible_race =
      this->are_dependent(nextSP, S_i);  //&& !this->happensBeforeThread(i, p);

  // If there exists i such that ...
  if (has_reversible_race) {
    std::unordered_set<tid_t> E;

    const std::unordered_set<runner_id_t> &enabled_at_preSi =
        preSi.get_enabled_runners();

    for (tid_t q : enabled_at_preSi) {
      const bool inE = q == p;  //|| this->threadsRaceAfterDepth(i, q, p);

      // If E != empty set
      if (inE && !preSi.sleep_set_contains(q)) E.insert(q);
    }

    if (E.empty()) {
      // E is the empty set -> add every enabled thread at pre(S, i)
      for (tid_t q : enabled_at_preSi)
        if (!preSi.sleep_set_contains(q))
          preSi.insert_into_backtrack_set_unless_completed(q);
    } else {
      for (tid_t q : E) {
        // If there is a thread in preSi that we
        // are already backtracking AND which is contained
        // in the set E, chose that thread to backtrack
        // on. This is equivalent to not having to do
        // anything
        if (preSi.backtrack_set_contains(q)) return true;
      }

      // Otherwise select an arbitrary thread to backtrack upon.
      preSi.insert_into_backtrack_set_unless_completed(*E.begin());
    }
  }
  return has_reversible_race;
}
