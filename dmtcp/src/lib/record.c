#include "mcmini/spy/checkpointing/record.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t rec_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pending_op_lock = PTHREAD_MUTEX_INITIALIZER;
enum libmcmini_mode libmcmini_mode = PRE_DMTCP;
visible_object empty_visible_obj = {.type = UNKNOWN, .location = NULL};
rec_list *head = NULL;
rec_list *current = NULL;
pending_operation *head_op = NULL;
pending_operation *current_op = NULL;

transition invisible_operation_for_this_thread(void) {
  transition t = {.type = INVISIBLE_OPERATION_TYPE, .executor = pthread_self()};
  return t;
}

rec_list *find_object(void *addr) {
  for (rec_list *node = head; node != NULL; node = node->next) {
    if (node->vo.location == addr) return node;
  }
  return NULL;
}

pending_operation *find_pending_op(pthread_t t) {
  for (pending_operation *node = head_op; node != NULL; node = node->next) {
    if (pthread_equal(t, node->t.executor)) return node;
  }
  return NULL;
}

rec_list *add_rec_entry(const visible_object *vo) {
  rec_list *new_node = (rec_list *)malloc(sizeof(rec_list));
  if (new_node == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  new_node->vo = *vo;
  if (head == NULL) {
    head = new_node;
    current = head;
  } else {
    current->next = new_node;
    current = new_node;
  }
  return new_node;
}

pending_operation *add_pending_op(const transition *t) {
  pending_operation *new_node =
      (pending_operation *)malloc(sizeof(pending_operation));
  if (new_node == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  new_node->t = *t;
  if (head_op == NULL) {
    head_op = new_node;
    current_op = head_op;
  } else {
    current_op->next = new_node;
    current_op = new_node;
  }
  return new_node;
}
