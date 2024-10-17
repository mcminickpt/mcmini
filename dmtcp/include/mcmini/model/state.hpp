#pragma once

#include <limits>
#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/misc/asserts.hpp"
#include "mcmini/model/defines.hpp"
#include "mcmini/model/runner_state.hpp"
#include "mcmini/model/visible_object.hpp"

namespace model {
class state {
 public:
  using objid_t = uint32_t;
  using runner_id_t = ::runner_id_t;

  virtual ~state() = default;
  virtual size_t count() const = 0;
  virtual size_t runner_count() const = 0;
  virtual bool is_runner(objid_t id) const = 0;
  virtual bool contains_object_with_id(objid_t id) const = 0;
  virtual bool contains_runner_with_id(runner_id_t id) const = 0;
  virtual objid_t get_objid_for_runner(runner_id_t id) const = 0;
  virtual runner_id_t get_runner_id_for_obj(objid_t id) const = 0;
  virtual const visible_object_state *get_state_of_object(objid_t id) const = 0;
  virtual const runner_state *get_state_of_runner(runner_id_t id) const = 0;
  virtual std::unique_ptr<mutable_state> mutable_clone() const = 0;
  virtual std::string debug_string() const { return ""; }

  // TODO: Potentially provide an interface here that conforms to C++11's
  // iteration (a begin() and end() as virtual functions perhaps).
  // Each subclass can return the same `state_iterator` type that
  // is defined elsewhere which should provide a pair of objid_t
  // and const visible_object_state* associated with that id.

  template <typename concrete_visible_object_state>
  const concrete_visible_object_state *get_state_of_object(objid_t id) const {
    static_assert(std::is_base_of<visible_object_state,
                                  concrete_visible_object_state>::value,
                  "Concrete type must be a subtype of `visible_object_state`");
    return static_cast<const concrete_visible_object_state *>(
        this->get_state_of_object(id));
  }

  template <typename concrete_visible_object_state>
  const concrete_visible_object_state *get_state_of_runner(
      runner_id_t id) const {
    static_assert(std::is_base_of<visible_object_state,
                                  concrete_visible_object_state>::value,
                  "Concrete type must be a subtype of `visible_object_state`");
    return static_cast<const concrete_visible_object_state *>(
        this->get_state_of_runner(id));
  }
};

class mutable_state : public state {
 public:
  virtual ~mutable_state() = default;

  /**
   * @brief Begin tracking a new visible object _obj_ to this state.
   *
   * @param initial_state the initial state of the object.
   * @return the new id that is assigned to the object. This id is unique from
   * every other id assigned to the objects in this state.
   */
  virtual objid_t add_object(const visible_object_state *initial_state) = 0;

  /**
   * @brief Begin tracking a new visible object, but consider it as the state of
   * a new runner.
   *
   * @return the id of the runner that was just added. This is NOT the same as
   * the runner's object id.
   */
  virtual runner_id_t add_runner(const runner_state *initial_state) = 0;

  /**
   * @brief Adds the given state _state_ for the object with id _id_.
   *
   * @note: If the object with id `id` is _not_ a visible object tracking states
   * of type `visible_object_state_type`, the behavior of this function is
   * undefined.
   */
  virtual void add_state_for_obj(objid_t id, const visible_object_state *) = 0;

  /**
   * @brief Adds the given state _state_ for the runner with id _id_.
   *
   * This is equivalent to first retrieving the object id of the runner in this
   * state and then asking for that object's state.
   */
  virtual void add_state_for_runner(runner_id_t id, const runner_state *) = 0;

  /**
   * @brief Creates a copy of the given state.
   *
   * All visible objects in the underlying state are copied into the new state
   * and are independently modifiable with respect to the first state.
   */
  std::unique_ptr<mutable_state> clone() const { return this->mutable_clone(); }

  template <typename concrete_visible_object_state>
  const concrete_visible_object_state *get_state_of_object(objid_t id) const {
    return (static_cast<const state *>(this))
        ->get_state_of_object<concrete_visible_object_state>(id);
  }

  template <typename concrete_visible_object_state>
  const concrete_visible_object_state *get_state_of_runner(
      runner_id_t id) const {
    static_assert(std::is_base_of<visible_object_state,
                                  concrete_visible_object_state>::value,
                  "Concrete type must be a subtype of `visible_object_state`");
    return (static_cast<const state *>(this))
        ->get_state_of_runner<concrete_visible_object_state>(id);
  }
};

constexpr static auto invalid_objid =
    std::numeric_limits<state::objid_t>::max();
constexpr static auto invalid_rid =
    std::numeric_limits<state::runner_id_t>::max();

}  // namespace model
