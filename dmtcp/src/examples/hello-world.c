#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int DEBUG = 0;

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
  pthread_mutex_t *dining_fork;
};

void *philosopher_doit(void *forks_arg) {
  struct forks *forks = forks_arg;
  sleep(1);
  pthread_mutex_lock(forks->dining_fork);
  pthread_mutex_lock(forks->left_fork);
  printf("Philosopher %d just ate\n", forks->philosopher);
  sleep(2);
  pthread_mutex_lock(forks->right_fork);
  pthread_mutex_unlock(forks->dining_fork);
  sleep(2);

  if (DEBUG) printf("Philosopher %d is eating.\n", forks->philosopher);

  pthread_mutex_unlock(forks->left_fork);
  pthread_mutex_unlock(forks->right_fork);
  return NULL;
}

int main(int argc, char *argv[]) {
  int NUM_THREADS = 2;
  DEBUG = 0;

  pthread_t thread[NUM_THREADS];
  pthread_mutex_t mutex_resource[NUM_THREADS];
  struct forks forks[NUM_THREADS];

  pthread_mutex_t dining_fork;
  pthread_mutex_init(&dining_fork, NULL);

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_mutex_init(&mutex_resource[i], NULL);
    forks[i] =
        (struct forks){i, &mutex_resource[i],
                       &mutex_resource[(i + 1) % NUM_THREADS], &dining_fork};
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
  }

  return 0;
}
