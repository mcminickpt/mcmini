#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#undef assert(x)
#define assert(x)                 \
  do {                            \
    if (!(x)) { raise(SIGUSR2); } \
  } while (0);

pthread_mutex_t mut;
pthread_cond_t cond;
int num_elements_to_consume = 0;
int number_consumed         = 0;

void
do_consume()
{
  pthread_mutex_lock(&mut);
  if (num_elements_to_consume == 0) {
    pthread_cond_wait(&cond, &mut);
  }
  pthread_mutex_unlock(&mut);

  pthread_mutex_lock(&mut);
  assert(num_elements_to_consume > 0);
  num_elements_to_consume--;
  pthread_mutex_unlock(&mut);
}

void
consumer(void)
{
  while (1) {
    do_consume();
    number_consumed++;
  }
}

void
producer()
{
  while (1) {
    pthread_mutex_lock(&mut);
    num_elements_to_consume++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mut);
  }
}

int
main()
{
  pthread_t pro[5], con[5];
  pthread_mutex_init(&mut, NULL);
  pthread_cond_init(&cond, NULL);

  int a[5] = {1, 2, 3, 4,
              5}; // Just used for numbering the producer and consumer

  for (int i = 0; i < 1; i++) {
    pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
  }
  for (int i = 0; i < 1; i++) {
    pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
  }

  for (int i = 0; i < 1; i++) { pthread_join(pro[i], NULL); }
  for (int i = 0; i < 1; i++) { pthread_join(con[i], NULL); }

  return 0;
}
