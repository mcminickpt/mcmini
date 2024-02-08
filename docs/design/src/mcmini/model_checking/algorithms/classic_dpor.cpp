#include "mcmini/model_checking/algorithms/classic_dpor.hpp"

#include <unistd.h>

#include <iostream>
#include <stack>
#include <unordered_set>

#include "mcmini/model/program.hpp"

using namespace model_checking;
using namespace model;

struct transition_sequence_entry {
  std::unordered_set<program::runner_id_t> backtrack_set;
  std::unordered_set<program::runner_id_t> sleep_set;
};

void classic_dpor::verify_using(coordinator &coordinator,
                                const callbacks &callbacks) {
  std::cout << "DPOR is running here! Yay!!" << std::endl;

  std::stack<transition_sequence_entry> dpor_specific_items;

  // Keep track of stuff here...

  // auto enabled_runners =
  //     coordinator.get_current_program_model().get_enabled_runners();

  // TODO: We could attach a `model_checking::oracle` here which, given
  // a set of threads and a program trace, perhaps some other information,
  // decides which thread to run. Something like this might be interesting here.

  // Pick an enabled thread etc.

  // Based on the items in `dpor_specific_items`, do something interesting.

  // For now, we simply tell the coordinator to run one thread for a few steps,
  // backtrack once, and then exit
  coordinator.execute_runner(0);
  coordinator.execute_runner(0);
}

// void classic_dpor::dynamicallyUpdateBacktrackSets() {
//   /*
//    * Updating the backtrack sets is accomplished as follows
//    * (under the given assumptions)
//    *
//    * ASSUMPTIONS
//    *
//    * 1. The state reflects last(S) for the transition stack
//    *
//    * 2. The thread that ran last is at the top of the transition
//    * stack (this should always be true)
//    *
//    * 3. The next transition for the thread that ran the most
//    * recent transition in the transition stack (the transition at the
//    * top of the stack) has been properly updated to reflect what that
//    * thread will do next
//    *
//    * WLOG, assume there are `n` transitions in the transition stack
//    * and `k` threads that are known to exist at the time of updating
//    * the backtrack sets. Note this implies that there are `n+1` items
//    * in the state stack (since there is always the initial state + 1
//    * for every subsequent transition thereafter)
//    *
//    * Let
//    *  S_i = ith backtracking state item
//    *  T_i = ith transition
//    *  N_p = the next transition for thread p (next(s, p))
//    *
//    * ALGORITHM:
//    *
//    * 1. First, get a reference to the transition at the top
//    * of the transition stack (i.e. the most recent transition)
//    * as well as the thread that ran that transition. WLOG suppose that
//    * thread has a thread id `i`.
//    *
//    * This transition will be used to test against the transitions
//    * queued as running "next" for all of the **other** threads
//    * that exist
//    *
//    *  2. Test whether a backtrack point is needed at state
//    *  S_n for the other threads by comparing N_p, for all p != i.
//    *
//    *  3. Get a reference to N_i and traverse the transition stack
//    *  to determine if a backtrack point is needed anywhere for
//    *  thread `i`
//    */
//   const uint64_t num_threads = this->getNumProgramThreads();

//   std::unordered_set<tid_t> thread_ids;
//   for (tid_t i = 0; i < num_threads; i++) thread_ids.insert(i);

//   // 3. Determine the i
//   const MCTransition &tStackTop = this->getTransitionStackTop();
//   const tid_t mostRecentThreadId = tStackTop.getThreadId();
//   const MCTransition &nextTransitionForMostRecentThread =
//       this->getNextTransitionForThread(mostRecentThreadId);
//   thread_ids.erase(mostRecentThreadId);

//   // O(# threads)
//   {
//     const MCTransition &S_n = this->getTransitionStackTop();
//     MCStackItem &s_n = this->getStateItemAtIndex(this->transitionStackTop);
//     const std::unordered_set<tid_t> enabledThreadsAt_s_n =
//         s_n.getEnabledThreadsInState();

//     for (tid_t tid : thread_ids) {
//       const MCTransition &nextSP = this->getNextTransitionForThread(tid);
//       this->dynamicallyUpdateBacktrackSetsHelper(
//           S_n, s_n, nextSP, this->transitionStackTop, (int)tid);
//     }
//   }

//   // O(transition stack size)

//   // It only remains to add backtrack points at the necessary
//   // points for thread `mostRecentThreadId`. We start at one step
//   // below the top since we know that transition to not be co-enabled
//   // (since it was, by assumption, run by `mostRecentThreadId`
//   for (int i = this->transitionStackTop - 1; i >= 0; i--) {
//     const MCTransition &S_i = this->getTransitionAtIndex(i);
//     MCStackItem &preSi = this->getStateItemAtIndex(i);
//     const bool shouldStop = dynamicallyUpdateBacktrackSetsHelper(
//         S_i, preSi, nextTransitionForMostRecentThread, i,
//         (int)mostRecentThreadId);
//     /*
//      * Stop when we find the first such i; this
//      * will be the maxmimum `i` since we're searching
//      * backwards
//      */
//     if (shouldStop) break;
//   }
// }

// bool MCStack::dynamicallyUpdateBacktrackSetsHelper(const MCTransition &S_i,
//                                                    MCStackItem &preSi,
//                                                    const MCTransition
//                                                    &nextSP, int i, int p) {
//   const unordered_set<tid_t> enabledThreadsAtPreSi =
//       preSi.getEnabledThreadsInState();
//   const bool shouldProcess = MCTransition::dependentTransitions(S_i, nextSP)
//   &&
//                              MCTransition::coenabledTransitions(S_i, nextSP)
//                              && !this->happensBeforeThread(i, p);

//   // if there exists i such that ...
//   if (shouldProcess) {
//     std::unordered_set<tid_t> E;

//     for (tid_t q : enabledThreadsAtPreSi) {
//       const bool inE = q == p || this->threadsRaceAfterDepth(i, q, p);
//       const bool isInSleepSet = preSi.threadIsInSleepSet(q);

//       // If E != empty set
//       if (inE && !isInSleepSet) E.insert(q);
//     }

//     if (E.empty()) {
//       // E is the empty set -> add every enabled thread at pre(S, i)
//       for (tid_t q : enabledThreadsAtPreSi)
//         if (!preSi.threadIsInSleepSet(q))
//           preSi.addBacktrackingThreadIfUnsearched(q);
//     } else {
//       for (tid_t q : E) {
//         // If there is a thread in preSi that we
//         // are already backtracking AND which is contained
//         // in the set E, chose that thread to backtrack
//         // on. This is equivalent to not having to do
//         // anything
//         if (preSi.isBacktrackingOnThread(q)) return shouldProcess;
//       }
//       preSi.addBacktrackingThreadIfUnsearched(*E.begin());
//     }
//   }
//   return shouldProcess;
// }
