#pragma once

#include "mcmini/model/objects/semaphore.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct sem_init : public model::transition {
 private:
  const state::objid_t sem_id;
  unsigned count = 0;

 public:
  sem_init(runner_id_t executor, state::objid_t sem_id)
      : sem_init(executor, sem_id, 0) {}
  sem_init(runner_id_t executor, state::objid_t sem_id, unsigned count)
      : transition(executor), sem_id(sem_id), count(count) {}
  ~sem_init() = default;
  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    s.add_state_for_obj(sem_id, new semaphore(count));
    return status::exists;
  }
  state::objid_t get_id() const { return this->sem_id; }
  std::string to_string() const override {
    return "sem_init(semaphore:" + std::to_string(sem_id) + ")";
  }
};
}  // namespace transitions
}  // namespace model
