#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_READERS 3
#define NUM_WRITERS 3
#define NUM_LOOP 2

pthread_rwlock_t rw;


void *reader(void *notused) {
    for(int i=0; i< NUM_LOOP; i++) {
        // acquire resource
        pthread_rwlock_rdlock(&rw);

        // use resource (we fake this by sleeping)
        printf("reader is reading\n");
        sleep(1);
        // release resource
        pthread_rwlock_unlock(&rw);
    }
    return NULL;
}

void *writer(void *notused) {
    for(int i=0; i< NUM_LOOP; i++) {
        // acquire resource
        pthread_rwlock_wrlock(&rw);
        
        // use resource (we fake this by sleeping)
        printf("writer is writing\n");
        sleep(5);
        // release resource
        pthread_mutex_unlock(&rw);
    }
    return NULL;
}

int main() {
    pthread_t read_thread[NUM_READERS];
    pthread_t write_thread[NUM_WRITERS];
    pthread_rwlock_init(&rw);
    
    int i;
    for (i = 0; i < NUM_READERS; i++) {
        pthread_create(&read_thread[i], NULL, reader, NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&write_thread[i], NULL, writer, NULL);
    }

    for (i = 0; i < NUM_READERS; i++) {
        pthread_join(read_thread[i], NULL);
    }
    for (i = 0; i < NUM_WRITERS; i++) {
        pthread_join(write_thread[i], NULL);
    }
    return 0;
}
