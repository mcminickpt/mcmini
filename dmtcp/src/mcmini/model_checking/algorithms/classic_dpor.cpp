#include "mcmini/model_checking/algorithms/classic_dpor.hpp"

#include <sys/types.h>

#include <array>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "mcmini/coordinator/coordinator.hpp"
#include "mcmini/defines.h"
#include "mcmini/model/exception.hpp"
#include "mcmini/model/program.hpp"
#include "mcmini/model/transition.hpp"
#include "mcmini/model/transitions/condition_variables/callbacks.hpp"
#include "mcmini/model/transitions/mutex/callbacks.hpp"
#include "mcmini/model/transitions/mutex/mutex_init.hpp"
#include "mcmini/model/transitions/semaphore/callbacks.hpp"
#include "mcmini/model/transitions/thread/callbacks.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor/clock_vector.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor/runner_item.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor/stack_item.hpp"
#include "mcmini/real_world/process.hpp"

using namespace model;
using namespace model_checking;

struct classic_dpor::dpor_context {
  ::coordinator &coordinator;
  std::vector<model_checking::stack_item> stack;

  dpor_context(::coordinator &c) : coordinator(c) {}

  const transition *get_transition(int i) const {
    return stack.at(i).get_out_transition();
  }
};

clock_vector classic_dpor::accumulate_max_clock_vector_against(
    const model::transition &t, const dpor_context &context) const {
  // The last state in the stack does NOT have an out transition, hence the
  // `nullptr` check. Note that `s_i.get_out_transition()` refers to `S_i`
  // (case-sensitive) in the paper, viz. the transition between states `s_i` and
  // `s_{i+1}`.
  clock_vector result;
  for (const stack_item &s_i : context.stack) {
    if (s_i.get_out_transition() != nullptr &&
        this->are_dependent(*s_i.get_out_transition(), t)) {
      result = clock_vector::max(result, s_i.get_clock_vector());
    }
  }
  return result;
}

bool classic_dpor::are_coenabled(const model::transition &t1,
                                 const model::transition &t2) const {
  return t1.get_executor() != t2.get_executor() &&
         this->config.coenabled_relation.call_or(true, &t1, &t2);
}

bool classic_dpor::are_dependent(const model::transition &t1,
                                 const model::transition &t2) const {
  return t1.get_executor() == t2.get_executor() ||
         this->config.dependency_relation.call_or(true, &t1, &t2);
}

bool classic_dpor::happens_before(const dpor_context &context, size_t i,
                                  size_t j) const {
  const runner_id_t rid =
      context.stack.at(i).get_out_transition()->get_executor();
  const clock_vector &cv = context.stack.at(j).get_clock_vector();
  return i <= cv.value_for(rid);
}

bool classic_dpor::happens_before_thread(const dpor_context &context, size_t i,
                                         runner_id_t p) const {
  const runner_id_t rid = context.get_transition(i)->get_executor();
  const clock_vector &cv =
      context.stack.back().get_per_runner_clocks()[p].get_clock_vector();
  return i <= cv.value_for(rid);
}

bool classic_dpor::threads_race_after(const dpor_context &context, size_t i,
                                      runner_id_t q, runner_id_t p) const {
  const size_t transition_stack_height = context.stack.size() - 1;
  for (size_t j = i + 1; j < transition_stack_height; j++) {
    if (q == context.get_transition(j)->get_executor() &&
        this->happens_before_thread(context, j, p))
      return true;
  }
  return false;
}

void classic_dpor::verify_using(coordinator &coordinator,
                                const callbacks &callbacks) {
  // The code below is an implementation of the model-checking algorithm of
  // Flanagan and Godefroid from 2015.

  // 1. Data structure set up

  /// @invariant: The number of items in the DPOR-specific stack is the same
  /// size as the number of transitions in the current trace plus one.
  ///
  /// The initial entry into the stack represents the information DPOR tracks
  /// for state `s_0`.
  stats model_checking_stats;
  dpor_context context(coordinator);
  auto &dpor_stack = context.stack;
  dpor_stack.emplace_back(
      clock_vector(),
      coordinator.get_current_program_model().get_enabled_runners());

  while (!dpor_stack.empty()) {
    // 2. Exploration phase
    while (dpor_stack.back().has_enabled_runners()) {
      if (dpor_stack.size() >= this->config.maximum_total_execution_depth) {
        throw std::runtime_error(
            "*** Execution Limit Reached! ***\n\n"
            "McMini ran a trace with" +
            std::to_string(dpor_stack.size()) +
            " transitions which is\n"
            "the more than McMini was configured to handle in any one trace (" +
            std::to_string(this->config.maximum_total_execution_depth) +
            "). Try running mcmini with the \"--max-depth-per-thread\" flag\n"
            "to limit how far into a trace McMini can go\n");
      }
      // NOTE: For deterministic results, always choose the "first" enabled
      // runner. A runner precedes another runner in being enabled iff it has a
      // smaller id.
      try {
        const runner_id_t rid = dpor_stack.back().get_first_enabled_runner();
        this->continue_dpor_by_expanding_trace_with(rid, context);
        model_checking_stats.total_transitions++;

        // Now ask the question: will the next operation of this thread
        // cause the program to exit or abort abnormally?
        //
        // If so, stop expanding the trace and backtrack. The rationale is that
        // any extension of this trace will eventually show the same bug anyway
        // (the transition claims to cause the program to abort), and smaller
        // traces are more understandable.
        const transition *rtransition =
            coordinator.get_current_program_model().get_pending_transition_for(
                rid);
        if (rtransition->aborts_program_execution()) {
          throw real_world::process::termination_error(SIGABRT, rid,
                                                       "The program aborted");
        } else if (rtransition->program_exit_code() > 0) {
          throw real_world::process::nonzero_exit_code_error(
              rtransition->program_exit_code(), rid,
              "The program exited abnormally");
        }
      } catch (const model::undefined_behavior_exception &ube) {
        if (callbacks.undefined_behavior)
          callbacks.undefined_behavior(coordinator, model_checking_stats, ube);
        return;
      } catch (const real_world::process::termination_error &te) {
        if (callbacks.abnormal_termination)
          callbacks.abnormal_termination(coordinator, model_checking_stats, te);
        return;
      } catch (const real_world::process::nonzero_exit_code_error &nzec) {
        if (callbacks.nonzero_exit_code)
          callbacks.nonzero_exit_code(coordinator, model_checking_stats, nzec);
        return;
      }
    }

    if (callbacks.trace_completed)
      callbacks.trace_completed(coordinator, model_checking_stats);

    if (callbacks.deadlock &&
        coordinator.get_current_program_model().is_in_deadlock())
      callbacks.deadlock(coordinator, model_checking_stats);

    // 3. Backtrack phase
    while (!dpor_stack.empty() && dpor_stack.back().backtrack_set_empty())
      dpor_stack.pop_back();

    model_checking_stats.trace_id++;
    if (!dpor_stack.empty()) {
      // At this point, the model checker's data structures are valid for
      // `dpor_stack.size()` states; however, the model and the associated
      // process(es) that the model represent do not yet correspond after
      // backtracking.
      coordinator.return_to_depth(dpor_stack.size() - 1);

      // The first step of the NEXT exploration phase begins with following
      // one of the backtrack threads. Select one thread to backtrack upon and
      // follow it before continuing onto the exploration phase.
      try {
        this->continue_dpor_by_expanding_trace_with(
            dpor_stack.back().backtrack_set_pop_first(), context);
      } catch (const model::undefined_behavior_exception &ube) {
        if (callbacks.undefined_behavior)
          callbacks.undefined_behavior(coordinator, model_checking_stats, ube);
        return;
      }
    }
  }
}

void classic_dpor::continue_dpor_by_expanding_trace_with(
    runner_id_t p, dpor_context &context) {
  context.coordinator.execute_runner(p);
  this->grow_stack_after_running(context);
  this->dynamically_update_backtrack_sets(context);
}

void classic_dpor::grow_stack_after_running(dpor_context &context) {
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
  const coordinator &coordinator = context.coordinator;
  assert(coordinator.get_depth_into_program() == context.stack.size());
  const model::transition *t_n =
      coordinator.get_current_program_model().get_trace().back();

  // NOTE: `cv` corresponds to line 14.3 of figure 4 in the DPOR paper.
  clock_vector cv = accumulate_max_clock_vector_against(*t_n, context);

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
  auto s_n_per_runner_clocks = context.stack.back().get_per_runner_clocks();
  s_n_per_runner_clocks[t_n->get_executor()].set_clock_vector(cv);
  context.stack.emplace_back(
      cv, std::move(s_n_per_runner_clocks), t_n,
      coordinator.get_current_program_model().get_enabled_runners());
  stack_item &s_n = context.stack.at(context.stack.size() - 2);
  stack_item &s_n_plus_1 = context.stack.back();

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
  s_n.set_out_transition(t_n);
  s_n.insert_into_sleep_set(t_n->get_executor());
  s_n.mark_searched(t_n->get_executor());
}

void classic_dpor::dynamically_update_backtrack_sets(dpor_context &context) {
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
  const coordinator &coordinator = context.coordinator;
  const size_t num_threads =
      coordinator.get_current_program_model().get_num_runners();

  std::set<runner_id_t> thread_ids;
  for (runner_id_t i = 0; i < num_threads; i++) thread_ids.insert(i);

  const ssize_t t_stack_top = (ssize_t)(context.stack.size()) - 2;
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

    for (runner_id_t rid : thread_ids) {
      const model::transition &next_sp =
          *coordinator.get_current_program_model()
               .get_pending_transitions()
               .get_transition_for_runner(rid);
      dynamically_update_backtrack_sets_at_index(context, S_n, next_sp,
                                                 context.stack.at(t_stack_top),
                                                 t_stack_top, rid);
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
    for (int i = t_stack_top - 1; i >= 0; i--) {
      const model::transition &S_i =
          *coordinator.get_current_program_model().get_trace().at(i);
      const bool should_stop = dynamically_update_backtrack_sets_at_index(
          context, S_i, next_s_p_for_latest_runner, context.stack.at(i), i,
          last_runner_to_execute);
      /*
       * Stop when we find the _first_ such i; this
       * will be the maxmimum `i` since we're searching
       * backwards
       */
      if (should_stop) break;
    }
  }
}

bool classic_dpor::dynamically_update_backtrack_sets_at_index(
    const dpor_context &context, const model::transition &S_i,
    const model::transition &next_sp, stack_item &pre_si, size_t i,
    runner_id_t p) {
  // TODO: add in co-enabled conditions
  const bool has_reversible_race = this->are_dependent(next_sp, S_i) &&
                                   this->are_coenabled(next_sp, S_i) &&
                                   !this->happens_before_thread(context, i, p);

  // If there exists i such that ...
  if (has_reversible_race) {
    std::set<runner_id_t> e;

    for (runner_id_t const q : pre_si.get_enabled_runners()) {
      const bool in_e = q == p || this->threads_race_after(context, i, q, p);

      // If E != empty set
      if (in_e && !pre_si.sleep_set_contains(q)) e.insert(q);
    }

    if (e.empty()) {
      // E is the empty set -> add every enabled thread at pre(S, i)
      for (runner_id_t const q : pre_si.get_enabled_runners())
        if (!pre_si.sleep_set_contains(q))
          pre_si.insert_into_backtrack_set_unless_completed(q);
    } else {
      for (runner_id_t const q : e) {
        // If there is a thread in preSi that we
        // are already backtracking AND which is contained
        // in the set E, chose that thread to backtrack
        // on. This is equivalent to not having to do
        // anything
        if (pre_si.backtrack_set_contains(q)) return true;
      }

      // Otherwise select an arbitrary thread to backtrack upon.
      pre_si.insert_into_backtrack_set_unless_completed(*e.begin());
    }
  }
  return has_reversible_race;
}

classic_dpor::dependency_relation_type classic_dpor::default_dependencies() {
  classic_dpor::dependency_relation_type dr;
  dr.register_dd_entry<const transitions::thread_create>(
      &transitions::thread_create::depends);
  dr.register_dd_entry<const transitions::thread_join>(
      &transitions::thread_join::depends);
  dr.register_dd_entry<const transitions::mutex_lock,
                       const transitions::mutex_init>(
      &transitions::mutex_lock::depends);
  dr.register_dd_entry<const transitions::mutex_lock,
                       const transitions::mutex_lock>(
      &transitions::mutex_lock::depends);
  dr.register_dd_entry<const transitions::condition_variable_wait,
                       const transitions::condition_variable_init>(
      &transitions::condition_variable_wait::depends);
  dr.register_dd_entry<const transitions::condition_variable_wait,
                       const transitions::mutex_lock>(
      &transitions::condition_variable_wait::depends);
  dr.register_dd_entry<const transitions::condition_variable_signal,
                       const transitions::condition_variable_wait>(
      &transitions::condition_variable_signal::depends);
  dr.register_dd_entry<const transitions::condition_variable_signal,
                       const transitions::mutex_lock>(
      &transitions::condition_variable_signal::depends);
  return dr;
}

classic_dpor::coenabled_relation_type classic_dpor::default_coenabledness() {
  classic_dpor::dependency_relation_type cr;
  cr.register_dd_entry<const transitions::thread_create>(
      &transitions::thread_create::coenabled_with);
  cr.register_dd_entry<const transitions::thread_join>(
      &transitions::thread_join::coenabled_with);
  cr.register_dd_entry<const transitions::mutex_lock,
                       const transitions::mutex_unlock>(
      &transitions::mutex_lock::coenabled_with);
  cr.register_dd_entry<const transitions::condition_variable_signal,
                       const transitions::condition_variable_wait>(
      &transitions::condition_variable_signal::coenabled_with);
  cr.register_dd_entry<const transitions::condition_variable_signal,
                       const transitions::mutex_unlock>(
      &transitions::condition_variable_signal::coenabled_with);
  cr.register_dd_entry<const transitions::condition_variable_broadcast,
                       const transitions::condition_variable_wait>(
      &transitions::condition_variable_broadcast::coenabled_with);
  cr.register_dd_entry<const transitions::condition_variable_broadcast,
                       const transitions::mutex_unlock>(
      &transitions::condition_variable_broadcast::coenabled_with);
  cr.register_dd_entry<const transitions::condition_variable_destroy,
                       const transitions::condition_variable_wait>(
      &transitions::condition_variable_destroy::coenabled_with);
  cr.register_dd_entry<const transitions::condition_variable_destroy,
                       const transitions::condition_variable_signal>(
      &transitions::condition_variable_destroy::coenabled_with);
  return cr;
}
