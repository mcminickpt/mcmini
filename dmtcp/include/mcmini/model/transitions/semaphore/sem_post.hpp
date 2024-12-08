#pragma once

#include "mcmini/model/objects/semaphore.hpp"
#include "mcmini/model/transition.hpp"

namespace model {
namespace transitions {

struct sem_post : public model::transition {
 private:
  const state::objid_t sem_id;

 public:
  sem_post(runner_id_t executor, state::objid_t sem_id)
      : transition(executor), sem_id(sem_id) {}
  ~sem_post() = default;
  status modify(model::mutable_state& s) const override {
    using namespace model::objects;
    const semaphore* sem = s.get_state_of_object<semaphore>(sem_id);
    s.add_state_for_obj(sem_id, new semaphore(sem->count() + 1));
    return status::exists;
  }
  state::objid_t get_id() const { return this->sem_id; }
  std::string to_string() const override {
    return "sem_post(semaphore:" + std::to_string(sem_id) + ")";
  }
};
}  // namespace transitions
}  // namespace model
