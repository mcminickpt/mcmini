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
  void verify_using(coordinator &, const callbacks &) override;

  void verify_using(coordinator &coordinator) {
    callbacks no_callbacks;
    this->verify_using(coordinator, no_callbacks);
  }

 private:
  double_dispatch_member_function_table<const model::transition, bool(void)>
      dependency_relation;

  bool are_dependent(const model::transition &t1,
                     const model::transition &t2) const {
    return t1.get_executor() == t2.get_executor() ||
           this->dependency_relation.call_or(true, &t1, &t2);
  }

  bool are_independent(const model::transition &t1,
                       const model::transition &t2) const {
    return !are_dependent(t1, t2);
  }

  // Do not call these methods directly. They are implementation details of
  // the DPOR algorithm and are called at specific points in time!

  clock_vector accumulate_max_clock_vector_against(
      const model::transition &, const std::vector<stack_item> &stack) const;

  void grow_stack_after_running(const coordinator &,
                                std::unordered_map<runner_id_t, runner_item> &,
                                std::vector<model_checking::stack_item> &);

  void dynamically_update_backtrack_sets(
      const coordinator &, std::vector<model_checking::stack_item> &);

  bool dynamically_update_backtrack_sets_at_index(
      const model::transition &S_i, const model::transition &nextSP,
      stack_item &preSi, int i, int p);
};

}  // namespace model_checking