#ifndef GMAL_GMALSHAREDTRANSITION_H
#define GMAL_GMALSHAREDTRANSITION_H

#include "GMALShared.h"
#include "GMALTransition.h"

struct GMALSharedTransition {
public:
    const std::type_info &type;
    tid_t executor;
    GMALSharedTransition(tid_t executor, const std::type_info &type) : executor(executor), type(type) {}
};

void GMALSharedTransitionReplace(GMALSharedTransition *, GMALSharedTransition *);

static_assert(std::is_trivially_copyable<GMALSharedTransition>::value,
        "The shared transition is not trivially copiable. Performing a memcpy of this type "
        "is undefined behavior according to the C++ standard. We need to add a fallback to "
        "writing the typeid hashcodes into shared memory and map to types instead"
        "This is currently unsupported at the moment");

#endif //GMAL_GMALSHAREDTRANSITION_H