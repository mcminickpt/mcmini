#ifndef MC_MCTHREADTRANSITIONWRAPPERS_H
#define MC_MCTHREADTRANSITIONWRAPPERS_H

#include <pthread.h>
#include "MCShared.h"

MC_EXTERN MC_NO_RETURN void mc_transparent_exit(int);
MC_EXTERN int mc_pthread_create(pthread_t *, const pthread_attr_t *, void *(*) (void *), void *);
MC_EXTERN int mc_pthread_join(pthread_t, void**);
MC_EXTERN void mc_pthread_reach_goal();
MC_EXTERN void mc_pthread_enter_goal_critical_section();
MC_EXTERN void mc_pthread_exit_goal_critical_section();

#define GOAL()               mc_pthread_reach_goal()
#define GOAL_ENTER_CRIT()    mc_pthread_enter_goal_critical_section()
#define GOAL_EXIT_CRIT()     mc_pthread_exit_goal_critical_section()

#endif //MC_MCTHREADTRANSITIONWRAPPERS_H
