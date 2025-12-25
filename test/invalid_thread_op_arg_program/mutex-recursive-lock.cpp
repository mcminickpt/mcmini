// BUG: the thread tries to recursively lock a mutex that it owns.
#include <pthread.h>

void* lock_thread(void* arg) {
    pthread_mutex_lock(&lock);
    pthread_mutex_lock(&lock);
    return NULL;
}
int main() {
    pthread_t t1, t2;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&t1, NULL, lock_thread, NULL);
    pthread_join(t1, NULL);
    pthread_mutex_destroy(&lock);
    return 0;
}
