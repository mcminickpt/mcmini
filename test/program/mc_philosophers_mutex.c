// Dining philosophers solution with mutex
#include "mcmini/mcmini.h"
#include "mcmini/mcmini_wrappers.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int DEBUG = 0;

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
  pthread_mutex_t *dining_fork;
};

void *philosopher_doit(void *forks_arg)
{
  struct forks *forks = forks_arg;
  mc_pthread_mutex_lock(forks->dining_fork);
  mc_pthread_mutex_lock(forks->left_fork);
  mc_pthread_mutex_lock(forks->right_fork);
  mc_pthread_mutex_unlock(forks->dining_fork);

  if (DEBUG)
    printf("Philosopher %d is eating.\n", forks->philosopher);

  mc_pthread_mutex_unlock(forks->left_fork);
  mc_pthread_mutex_unlock(forks->right_fork);
  return NULL;
}

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("Usage: %s NUM_PHILOSOPHERS DEBUG\n", argv[0]);
    return 1;
  }

  int NUM_PHILOSOPHERS = atoi(argv[1]);
  DEBUG = atoi(argv[2]);

  mcmini_main();
  pthread_t thread[NUM_PHILOSOPHERS];
  pthread_mutex_t mutex_resource[NUM_PHILOSOPHERS];
  struct forks philosophers[NUM_PHILOSOPHERS];

  pthread_mutex_t dining_fork;
  mc_pthread_mutex_init(&dining_fork, NULL);

  int i;
  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    mc_pthread_mutex_init(&mutex_resource[i], NULL);
    philosophers[i] = (struct forks){i, &mutex_resource[i],
                                     &mutex_resource[(i + 1) % NUM_PHILOSOPHERS],
                                     &dining_fork};
  }

  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    mc_pthread_create(&thread[i], NULL, &philosopher_doit, &philosophers[i]);
  }

  for (i = 0; i < NUM_PHILOSOPHERS; i++) {
    mc_pthread_join(thread[i], NULL);
  }

  return 0;
}
