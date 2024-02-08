#ifndef INCLUDE_MCMINI_MCSHAREDTRANSITION_HPP
#define INCLUDE_MCMINI_MCSHAREDTRANSITION_HPP

#include "mcmini/MCShared.h"
#include "mcmini/MCTransition.h"

struct MCSharedTransition {
public:

  const std::type_info &type;
  tid_t executor;
  MCSharedTransition(tid_t executor, const std::type_info &type)
    : executor(executor), type(type)
  {}
};

void MCSharedTransitionReplace(MCSharedTransition *,
                               MCSharedTransition *);

static_assert(MC_IS_TRIVIALLY_COPYABLE(MCSharedTransition),
              "The shared transition is not trivially copiable. "
              "Performing a memcpy of this type "
              "is undefined behavior according to the C++ standard. "
              "We need to add a fallback to "
              "writing the typeid hashcodes into shared memory and "
              "map to types instead"
              "This is currently unsupported at the moment");

#endif // INCLUDE_MCMINI_MCSHAREDTRANSITION_HPP
