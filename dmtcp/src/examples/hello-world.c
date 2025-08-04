// Naive dining philosophers solution, which leads to deadlock.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int DEBUG = 0;

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
} *forks;

void *philosopher_doit(void *forks_arg) {
  struct forks *forks = forks_arg;
  sleep(4);
  pthread_mutex_lock(forks->left_fork);
  sleep(1);
  pthread_mutex_lock(forks->right_fork);
  sleep(3);

  if (DEBUG) printf("Philosopher %d just ate.\n", forks->philosopher);

  pthread_mutex_unlock(forks->left_fork);
  sleep(1);
  pthread_mutex_unlock(forks->right_fork);
  return NULL;
}

int main(int argc, char *argv[]) {
  int NUM_THREADS = 3;
  DEBUG = 1;

  pthread_t thread[NUM_THREADS];
  pthread_mutex_t mutex_resource[NUM_THREADS];

  forks = malloc(NUM_THREADS * sizeof(struct forks));

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_mutex_init(&mutex_resource[i], NULL);
    forks[i] = (struct forks){i, &mutex_resource[i],
                              &mutex_resource[(i + 1) % NUM_THREADS]};
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
  }

  free(forks);
  return 0;
}
