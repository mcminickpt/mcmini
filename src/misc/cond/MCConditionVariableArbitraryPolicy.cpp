#include "mcmini/misc/cond/MCConditionVariableArbitraryPolicy.hpp"
#include <algorithm>
#include <memory>

namespace mcmini {

std::unique_ptr<ConditionVariablePolicy>
ConditionVariableArbitraryPolicy::clone() const
{
  return std::unique_ptr<ConditionVariableArbitraryPolicy>(
    new ConditionVariableArbitraryPolicy(*this));
}

void
ConditionVariableArbitraryPolicy::receive_signal_message()
{}

} // namespace mcmini