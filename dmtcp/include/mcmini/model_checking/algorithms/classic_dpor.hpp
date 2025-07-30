#pragma once

#include "mcmini/misc/ddt.hpp"
#include "mcmini/model_checking/algorithm.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor/runner_item.hpp"
#include "mcmini/model_checking/algorithms/classic_dpor/stack_item.hpp"

namespace model_checking {

/**
 * @brief A model-checking algorithm which performs verification using the
 * algorithm of Flanagan and Godefroid (2005).
 */
class classic_dpor final : public algorithm {
 public:
  using dependency_relation_type =
      double_dispatch_member_function_table<const model::transition,
                                            bool(void)>;

  using coenabled_relation_type =
      double_dispatch_member_function_table<const model::transition,
                                            bool(void)>;

  void verify_using(coordinator &, const callbacks &) override;
  void verify_using(coordinator &coordinator) {
    callbacks no_callbacks;
    this->verify_using(coordinator, no_callbacks);
  }
  static dependency_relation_type default_dependencies();
  static coenabled_relation_type default_coenabledness();

  struct configuration {
    uint32_t maximum_total_execution_depth = 1500;
  };

  classic_dpor(
      dependency_relation_type dependency_relation = default_dependencies(),
      coenabled_relation_type coenabled_relation = default_coenabledness())
      : dependency_relation(std::move(dependency_relation)),
        coenabled_relation(std::move(coenabled_relation)) {}

 private:
  double_dispatch_member_function_table<const model::transition, bool(void)>
      dependency_relation;

  double_dispatch_member_function_table<const model::transition, bool(void)>
      coenabled_relation;

  bool are_dependent(const model::transition &t1,
                     const model::transition &t2) const;

  bool are_coenabled(const model::transition &t1,
                     const model::transition &t2) const;

  bool are_independent(const model::transition &t1,
                       const model::transition &t2) const {
    return !are_dependent(t1, t2);
  }

  // Do not call these methods directly. They are implementation details of
  // the DPOR algorithm and are called at specific points in time!

  struct dpor_context;

  bool happens_before(const dpor_context &, size_t i, size_t j) const;
  bool happens_before_thread(const dpor_context &, size_t i,
                             runner_id_t p) const;
  bool threads_race_after(const dpor_context &context, size_t i, runner_id_t q,
                          runner_id_t p) const;

  clock_vector accumulate_max_clock_vector_against(const model::transition &,
                                                   const dpor_context &) const;

  void continue_dpor_by_expanding_trace_with(runner_id_t p, dpor_context &);
  void grow_stack_after_running(dpor_context &);
  void dynamically_update_backtrack_sets(dpor_context &);

  bool dynamically_update_backtrack_sets_at_index(
      const dpor_context &, const model::transition &S_i,
      const model::transition &nextSP, stack_item &preSi, size_t i,
      runner_id_t p);
};

}  // namespace model_checking
