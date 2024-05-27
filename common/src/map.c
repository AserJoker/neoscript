
#include "map.h"
#include "list.h"
#include <stdlib.h>
typedef struct _neo_map_pair {
  void *key;
  void *value;
  neo_free_fn keyfree;
  neo_free_fn valuefree;
} *neo_map_pair;
neo_map_pair create_neo_map_pair(neo_free_fn keyfree, neo_free_fn valuefree) {
  neo_map_pair pair = (neo_map_pair)malloc(sizeof(struct _neo_map_pair));
  pair->key = NULL;
  pair->value = NULL;
  pair->keyfree = keyfree;
  pair->valuefree = valuefree;
  return pair;
}
void free_neo_map_pair(neo_map_pair pair) {
  if (pair->keyfree && pair->key) {
    pair->keyfree(pair->key);
  }
  if (pair->valuefree && pair->value) {
    pair->valuefree(pair->value);
  }
  free(pair);
}
struct _neo_map {
  neo_compare_fn cmp;
  neo_free_fn keyfree;
  neo_free_fn valuefree;
  neo_list pairs;
};
neo_map create_neo_map(neo_compare_fn cmp, neo_free_fn keyfree,
                       neo_free_fn valuefree) {
  neo_map map = (neo_map)malloc(sizeof(struct _neo_map));
  map->keyfree = keyfree;
  map->valuefree = valuefree;
  map->cmp = cmp;
  map->pairs = create_neo_list((neo_free_fn)free_neo_map_pair);
  return map;
}
void free_neo_map(neo_map map) {
  free_neo_list(map->pairs);
  free(map);
}

neo_list neo_map_keys(neo_map self) {
  neo_list keys = create_neo_list(NULL);
  neo_list_node node = neo_list_head(self->pairs);
  while (node != neo_list_tail(self->pairs)) {
    neo_map_pair pair = (neo_map_pair)neo_list_node_get(node);
    if (pair) {
      neo_list_push(keys, pair->key);
    }
    node = neo_list_node_next(node);
  }
  return keys;
}
void *neo_map_get(neo_map self, void *key) {

  neo_list_node node = neo_list_head(self->pairs);
  while (node != neo_list_tail(self->pairs)) {
    neo_map_pair pair = (neo_map_pair)neo_list_node_get(node);
    if (pair && self->cmp(pair->key, key)) {
      return pair->value;
    }
    node = neo_list_node_next(node);
  }
  return NULL;
}
void neo_map_set(neo_map self, void *key, void *value) {
  neo_list_node node = neo_list_head(self->pairs);
  while (node != neo_list_tail(self->pairs)) {
    neo_map_pair pair = (neo_map_pair)neo_list_node_get(node);
    if (pair && self->cmp(pair->key, key)) {
      if (pair->value != value) {
        if (pair->value) {
          self->valuefree(pair->value);
          pair->value = value;
          return;
        }
      }
    }
    node = neo_list_node_next(node);
  }
  neo_map_pair pair = create_neo_map_pair(self->keyfree, self->valuefree);
  pair->key = key;
  pair->value = value;
  neo_list_push(self->pairs, pair);
}
int8_t neo_map_has(neo_map self, void *key) {

  neo_list_node node = neo_list_head(self->pairs);
  while (node != neo_list_tail(self->pairs)) {
    neo_map_pair pair = (neo_map_pair)neo_list_node_get(node);
    if (pair && self->cmp(pair->key, key)) {
      return 1;
    }
    node = neo_list_node_next(node);
  }
  return 0;
}
void neo_map_delete(neo_map self, void *key) {
  neo_list_node node = neo_list_head(self->pairs);
  while (node != neo_list_tail(self->pairs)) {
    neo_map_pair pair = (neo_map_pair)neo_list_node_get(node);
    if (pair && self->cmp(pair->key, key)) {
      neo_list_remove(self->pairs, node);
      return;
    }
    node = neo_list_node_next(node);
  }
  volatile int a;
}
size_t neo_map_length(neo_map self) { return neo_list_length(self->pairs); }