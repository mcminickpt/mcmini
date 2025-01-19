#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H
#include <pthread.h>
#include <stdbool.h>
#include "mcmini/defines.h"


typedef struct thread_queue_node {
    runner_id_t thread;
    struct thread_queue_node *next;
} thread_queue_node;

typedef struct thread_queue {
    thread_queue_node *front;
    thread_queue_node *rear;
    int size;
} thread_queue;

thread_queue_node* create_thread_queue_node();
thread_queue* create_thread_queue();
void init_thread_queue(thread_queue *queue);
void enqueue_thread(thread_queue *queue, runner_id_t thread);
runner_id_t dequeue_thread(thread_queue *queue);
runner_id_t peek_thread(thread_queue *queue);
bool is_in_thread_queue(thread_queue *queue, runner_id_t thread);
bool is_queue_empty(thread_queue *queue);
void print_thread_queue(const thread_queue *queue);

#endif // THREAD_QUEUE_H
