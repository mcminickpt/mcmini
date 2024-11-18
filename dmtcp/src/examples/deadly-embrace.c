// Naive dining philosophers solution, which leads to deadlock.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 2

pthread_mutex_t mut1;
pthread_mutex_t mut2;

void *philosopher_doit(void *unused) {
  sleep(4);
  pthread_mutex_lock(&mut2);
  pthread_mutex_lock(&mut1);
  pthread_mutex_unlock(&mut1);
  pthread_mutex_unlock(&mut2);
  return NULL;
}

int main(int argc, char *argv[]) {
  pthread_t child;
  pthread_mutex_t mutex_resource[NUM_THREADS];
  pthread_mutex_init(&mut1, NULL);
  pthread_mutex_init(&mut2, NULL);
  pthread_create(&child, NULL, &philosopher_doit, NULL);
  sleep(4);
  pthread_mutex_lock(&mut1);
  pthread_mutex_lock(&mut2);
  pthread_mutex_unlock(&mut1);
  pthread_mutex_unlock(&mut2);
  return 0;
}
