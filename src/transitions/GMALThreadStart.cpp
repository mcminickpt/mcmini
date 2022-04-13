//
// Created by parallels on 4/10/22.
//

#include "GMALThreadStart.h"

GMALTransition*
GMALReadThreadStart(void *shmStart, const GMALState &programState)
{
    std::shared_ptr<GMALThread> thread = programState.getThreadWithId(0);
    return new GMALThreadStart(thread);
}
