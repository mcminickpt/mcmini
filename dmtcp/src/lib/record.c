#include "mcmini/spy/checkpointing/record.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t rec_list_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pending_op_lock = PTHREAD_MUTEX_INITIALIZER;
enum libmcmini_mode libmcmini_mode = PRE_DMTCP;
visible_object empty_visible_obj = {.type = UNKNOWN, .location = NULL};
rec_list *head_record_mode = NULL;
rec_list *current_record_mode = NULL;
rec_list *head_after_restart = NULL;
rec_list *current_after_restart = NULL;

transition invisible_operation_for_this_thread(void) {
  transition t = {.type = INVISIBLE_OPERATION_TYPE, .executor = pthread_self()};
  return t;
}

rec_list *find_object(void *addr, rec_list *head) {
  for (rec_list *node = head; node != NULL; node = node->next) {
    if (node->vo.location == addr) return node;
  }
  return NULL;
}

rec_list *find_object_record_mode(void *addr) {
  return find_object(addr, head_record_mode);
}

rec_list *find_object_restart_mode(void *addr) {
  return find_object(addr, head_after_restart);
}

rec_list *add_rec_entry(const visible_object *vo, rec_list **head, rec_list **current) {
  rec_list *new_node = (rec_list *)malloc(sizeof(rec_list));
  if (new_node == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  new_node->vo = *vo;
  if (*head == NULL) {
    *head = new_node;
    *current = new_node;
  }
  else {
    (*current)->next = new_node;
    *current = new_node;
  }
  return new_node;
}

rec_list *add_rec_entry_record_mode(const visible_object *vo) {
  return add_rec_entry(vo, &head_record_mode, &current_record_mode);
}

rec_list *add_rec_entry_restart_mode(const visible_object *vo) {
  return add_rec_entry(vo, &head_after_restart, &current_after_restart);
}
