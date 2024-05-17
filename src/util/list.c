#include "util/list.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

struct _neo_list_node {
  void *data;
  neo_list_node next;
  neo_list_node last;
  neo_free_fn autofree;
};
struct _neo_list {
  neo_list_node head;
  neo_list_node tail;
  size_t size;
  neo_free_fn autofree;
};
neo_list_node create_neo_list_node(neo_list list) {
  neo_list_node node = (neo_list_node)malloc(sizeof(struct _neo_list_node));
  node->data = NULL;
  node->last = NULL;
  node->next = NULL;
  node->autofree = list->autofree;
  return node;
}
void free_neo_list_node(neo_list_node node) {
  if (node->next) {
    node->next->last = node->last;
  }
  if (node->last) {
    node->last->next = node->next;
  }
  if (node->autofree && node->data) {
    node->autofree(node->data);
  }
  free(node);
}
neo_list create_neo_list(neo_free_fn fn) {
  neo_list list = (neo_list)malloc(sizeof(struct _neo_list));
  assert(list != NULL);
  list->head = create_neo_list_node(list);
  list->tail = create_neo_list_node(list);
  list->head->next = list->tail;
  list->tail->last = list->head;
  list->size = 0;
  list->autofree = fn;
  return list;
}
void free_neo_list(neo_list list) {
  if (list) {
    while (list->head) {
      neo_list_node next = list->head->next;
      free_neo_list_node(list->head);
      list->head = next;
    }
    free(list);
    list = NULL;
  }
}

neo_list_node neo_list_head(neo_list self) { return self->head; }
neo_list_node neo_list_tail(neo_list self) { return self->tail; }

neo_list_node neo_list_node_next(neo_list_node node) { return node->next; }
neo_list_node neo_list_node_last(neo_list_node node) { return node->last; }

void *neo_list_node_get(neo_list_node self) { return self->data; }
void neo_list_node_set(neo_list_node self, void *data) { self->data = data; }

neo_list_node neo_list_push(neo_list self, void *data) {
  neo_list_node node = create_neo_list_node(self);
  node->data = data;
  node->last = self->tail->last;
  node->next = self->tail;
  self->tail->last->next = node;
  self->tail->last = node;
  self->size++;
  return node;
}
void *neo_list_pop(neo_list self) {
  if (self->size) {
    void *data = self->tail->last->data;
    self->tail->last->data = NULL;
    free_neo_list_node(self->tail->last);
    self->size--;
    return data;
  }
  return NULL;
}
neo_list_node neo_list_unshift(neo_list self, void *data) {

  neo_list_node node = create_neo_list_node(self);
  node->data = data;
  node->next = self->head->next;
  node->last = self->head;
  self->head->next->last = node;
  self->head->next = node;
  self->size++;
  return node;
}
void *neo_list_shift(neo_list self) {
  if (self->size) {
    neo_list_node node = self->head->next;
    void *data = node->data;
    node->data = NULL;
    free_neo_list_node(self->head->next);
    self->size--;
  }
  return NULL;
}

neo_list_node neo_list_insert(neo_list self, neo_list_node position,
                              void *data) {
  neo_list_node node = create_neo_list_node(self);
  node->data = data;
  node->last = position;
  node->next = position->next;
  position->next->last = node;
  position->next = node;
  self->size++;
  return node;
}

neo_list_node neo_list_find(neo_list self, void *data) {
  neo_list_node node = self->head;
  while (node != self->tail) {
    if (node->data == data) {
      return node;
    }
    node = node->next;
  }
  return NULL;
}
void neo_list_remove(neo_list self, neo_list_node position) {
  if (self->size) {
    free_neo_list_node(position);
    self->size--;
  }
}

const size_t neo_list_length(neo_list self) { return self->size; }
