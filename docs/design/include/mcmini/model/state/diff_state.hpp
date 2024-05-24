#pragma once

#include "mcmini/misc/injective_function.hpp"
#include "mcmini/model/state.hpp"
#include "mcmini/model/visible_object.hpp"

namespace model {

/// @brief A state which defines its own state as a change in state from some
/// base state `base_state`
class diff_state : public mutable_state {
 private:
  const state &base_state;

 public:
  injective_function<state::runner_id_t, state::objid_t> new_runners;
  std::unordered_map<state::objid_t, visible_object> new_object_states;

  diff_state(const state &s) : base_state(s) {}
  diff_state(const diff_state &ds)
      : base_state(ds.base_state), new_object_states(ds.new_object_states) {}
  diff_state(detached_state &&) = delete;
  diff_state &operator=(const diff_state &) = delete;
  detached_state &operator=(detached_state &&) = delete;

  const state &get_base() const { return this->base_state; }
  bool differs_from_base() const {
    return !this->new_object_states.empty() || !this->new_runners.empty();
  }

  /* `mutable_state` overrrides */
  size_t count() const override {
    size_t count = this->new_object_states.size() + base_state.count();
    for (const auto &p : new_object_states) {
      // Each item in `new_object_states` that is also in `base_state` defines
      // states for _previously existing_ objects. These objects are accounted
      // for in `count` (double-counted), hence the `--`
      if (base_state.contains_object_with_id(p.first)) count--;
    }
    return count;
  }
  size_t runner_count() const override {
    return base_state.runner_count() + new_runners.size();
  }
  objid_t get_objid_for_runner(runner_id_t id) const override;
  runner_id_t get_runner_id_for_obj(objid_t id) const override;
  bool is_runner(objid_t id) const override;
  bool contains_object_with_id(state::objid_t id) const override;
  bool contains_runner_with_id(runner_id_t id) const override;
  const visible_object_state *get_state_of_object(objid_t id) const override;
  const runner_state *get_state_of_runner(runner_id_t id) const override;
  objid_t add_object(const visible_object_state *) override;
  runner_id_t add_runner(const runner_state *) override;
  void add_state_for_obj(objid_t id, const visible_object_state *) override;
  void add_state_for_runner(runner_id_t id, const runner_state *) override;
  std::unique_ptr<mutable_state> mutable_clone() const override;
};
}  // namespace model
