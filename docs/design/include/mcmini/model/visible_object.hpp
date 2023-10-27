#pragma

#include <vector>

#include "mcmini/model/visible_object_state.hpp"

namespace mcmini::model {

/**
 *
 */
class visible_object {
 private:
  std::vector<std::unique_ptr<visible_object_state>>;

 public:
  visible_object(std::unique_ptr<visible_object_state> initial_state);
};
}  // namespace mcmini::model