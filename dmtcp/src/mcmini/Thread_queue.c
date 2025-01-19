#include "dmtcp/include/mcmini/Thread_queue.h"

#include <stdlib.h>
#include <stdio.h>

void init_thread_queue(thread_queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

void enqueue_thread(thread_queue *queue, pthread_t thread) {
    thread_queue_node *new_node = malloc(sizeof(thread_queue_node));
    if (!new_node) {
        perror("Failed to allocate memory for thread queue node");
        exit(EXIT_FAILURE);
    }
    new_node->thread = thread;
    new_node->next = NULL;

    if (queue->rear) {
        queue->rear->next = new_node;
    }
    queue->rear = new_node;

    if (!queue->front) {
        queue->front = new_node;
    }

    queue->size++;
}

pthread_t dequeue_thread(thread_queue *queue) {
    if (queue->front == NULL) {
        fprintf(stderr, "Error: Attempt to dequeue from an empty queue\n");
        exit(EXIT_FAILURE);
    }

    thread_queue_node *temp = queue->front;
    pthread_t thread = temp->thread;

    queue->front = queue->front->next;
    if (!queue->front) {
        queue->rear = NULL;
    }

    free(temp);
    queue->size--;
    return thread;
}

pthread_t peek_thread(thread_queue *queue) {
    if (queue->front == NULL) {
        fprintf(stderr, "Error: Attempt to peek at an empty queue\n");
        exit(EXIT_FAILURE);
    }
    return queue->front->thread;
}
