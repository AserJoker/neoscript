#include "util/imap.h"
#include "util/list.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct _neo_imap *neo_imap;
typedef struct _neo_imap_node *neo_imap_node;
struct _neo_imap {
  neo_list items;
  neo_free_fn autofree;
};
struct _neo_imap_node {
  void *data;
  int32_t key;
};
neo_imap create_neo_imap(neo_free_fn fn) {
  neo_imap imap = (neo_imap)malloc(sizeof(struct _neo_imap));
  imap->autofree = fn;
  imap->items = create_neo_list(free);
  return imap;
}
void free_neo_imap(neo_imap imap) {
  if (!imap) {
    return;
  }
  if (imap->autofree) {
    neo_list_node node = neo_list_head(imap->items);
    while (node != neo_list_tail(imap->items)) {
      neo_imap_node item = (neo_imap_node)neo_list_node_get(node);
      if (item) {
        imap->autofree(item->data);
      }
      node = neo_list_node_next(node);
    }
  }
  free_neo_list(imap->items);
  free(imap);
}
void neo_imap_set(neo_imap self, int32_t key, void *value) {
  neo_list_node node = neo_list_head(self->items);
  neo_imap_node item = NULL;
  while (node != neo_list_tail(self->items)) {
    neo_imap_node inode = (neo_imap_node)neo_list_node_get(node);
    if (inode && inode->key == key) {
      item = inode;
      break;
    }
    node = neo_list_node_next(node);
  }
  if (item) {
    if (self->autofree) {
      self->autofree(item->data);
      item->data = value;
    }
  } else {
    item = (neo_imap_node)malloc(sizeof(struct _neo_imap_node));
    assert(item != NULL);
    item->key = key;
    item->data = value;
    neo_list_push(self->items, item);
  }
}
void *neo_imap_get(neo_imap self, int32_t key) {
  neo_list_node node = neo_list_head(self->items);
  while (node != neo_list_tail(self->items)) {
    neo_imap_node inode = (neo_imap_node)neo_list_node_get(node);
    if (inode && inode->key == key) {
      return inode->data;
    }
    node = neo_list_node_next(node);
  }
  return NULL;
}
void neo_imap_delete(neo_imap self, int32_t key) {
  neo_list_node node = neo_list_head(self->items);
  while (node != neo_list_tail(self->items)) {
    neo_imap_node inode = (neo_imap_node)neo_list_node_get(node);
    if (inode && inode->key == key) {
      break;
    }
    node = neo_list_node_next(node);
  }
  if (node != neo_list_tail(self->items)) {
    neo_list_remove(self->items, node);
  }
}
size_t neo_imap_length(neo_imap self) { return neo_list_length(self->items); }