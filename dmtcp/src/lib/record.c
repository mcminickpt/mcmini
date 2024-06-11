#include "mcmini/spy/checkpointing/record.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t rec_list_lock = PTHREAD_MUTEX_INITIALIZER;
mode libmcmini_mode = TEMPLATE;
rec_list *head = NULL;
rec_list *current = NULL;

rec_list *find_mutex(pthread_mutex_t *mutex) {
  for (rec_list *node = head; node != NULL; node = node->next) {
    if (node->mutex == mutex) return node;
  }
  return NULL;
}

rec_list *add_rec_entry(pthread_mutex_t *mutex, mutex_state s) {
  rec_list *new_node = (rec_list *)malloc(sizeof(rec_list));
  if (new_node == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  new_node->state = s;
  new_node->mutex = mutex;
  new_node->next = NULL;

  if (head == NULL) {
    head = new_node;
    current = head;
  } else {
    current->next = new_node;
    current = new_node;
  }
  return new_node;
}
