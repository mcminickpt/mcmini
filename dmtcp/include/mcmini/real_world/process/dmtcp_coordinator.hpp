#pragma once

#include "mcmini/real_world/dmtcp_target.hpp"

namespace real_world {

struct dmtcp_coordinator : public dmtcp_target {
 public:
  dmtcp_coordinator();

 public:
  void launch_and_wait() override;
  uint32_t get_port() const { return this->port; }

 private:
  uint32_t port = 0;
};
}  // namespace real_world
