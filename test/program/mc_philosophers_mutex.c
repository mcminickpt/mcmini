// Dining philosophers solution with mutex

#include "mcmini/mcmini.h"
#include "mcmini/mcmini_wrappers.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 5

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
  pthread_mutex_t *dining_fork;
} forks[NUM_THREADS];

void *
philosopher_doit(void *forks_arg)
{
  struct forks *forks = forks_arg;
  mc_pthread_mutex_lock(forks->dining_fork);
  mc_pthread_mutex_lock(forks->left_fork);
  mc_pthread_mutex_lock(forks->right_fork);
  mc_pthread_mutex_unlock(forks->dining_fork);

  //  printf("Philosopher %d just ate.\n", forks->philosopher);
  mc_pthread_mutex_unlock(forks->left_fork);
  mc_pthread_mutex_unlock(forks->right_fork);
  return NULL;
}

int
main(int argc, char *argv[])
{
  mcmini_main();
  pthread_t thread[NUM_THREADS];
  pthread_mutex_t mutex_resource[NUM_THREADS];

  pthread_mutex_t dining_fork;
  mc_pthread_mutex_init(&dining_fork, NULL);

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    // ANSI C++ require the cast to pthread_mutex_t, 'struct forks',
    //  respectively, because these are runtime statements, and
    //  not declarations.  But C does not require the cast.
    //    mutex_resource[i] =
    //    (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    mc_pthread_mutex_init(&mutex_resource[i], NULL);
    forks[i] = (struct forks){i, &mutex_resource[i],
                              &mutex_resource[(i + 1) % NUM_THREADS],
                              &dining_fork};
  }

  for (i = 0; i < NUM_THREADS; i++) {
    mc_pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
  }

  for (i = 0; i < NUM_THREADS; i++) {
    mc_pthread_join(thread[i], NULL);
  }

  return 0;
}
