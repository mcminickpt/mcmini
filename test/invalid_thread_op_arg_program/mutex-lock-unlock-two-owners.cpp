// BUG: One thread locks a mutex, and a second thread unlocks the mutex.
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t lock;
sem_t signal_sem;

void* lock_thread(void* arg) {
    pthread_mutex_lock(&lock);
    sem_post(&signal_sem);
    return NULL;
}
void* unlock_thread(void* arg) {
    sem_wait(&signal_sem);
    pthread_mutex_unlock(&lock);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_mutex_init(&lock, NULL);
    sem_init(&signal_sem, 0, 0);
    pthread_create(&t1, NULL, lock_thread, NULL);
    pthread_create(&t2, NULL, unlock_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    sem_destroy(&signal_sem);
    pthread_mutex_destroy(&lock);
    return 0;
}
