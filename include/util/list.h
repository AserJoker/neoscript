#ifndef __NEO_UTIL_LIST__
#define __NEO_UTIL_LIST__
#include <stddef.h>
typedef struct _neo_list *neo_list;
typedef struct _neo_list_node *neo_list_node;

typedef void (*neo_free_fn)(void *);

neo_list create_neo_list(neo_free_fn fn);
void free_neo_list(neo_list list);

neo_list_node neo_list_head(neo_list self);
neo_list_node neo_list_tail(neo_list self);

neo_list_node neo_list_node_next(neo_list_node node);
neo_list_node neo_list_node_last(neo_list_node node);

neo_list_node neo_list_push(neo_list self, void *data);
void *neo_list_pop(neo_list self);
neo_list_node neo_list_unshift(neo_list self, void *data);
void *neo_list_shift(neo_list self);

neo_list_node neo_list_insert(neo_list self, neo_list_node position,
                              void *data);
neo_list_node neo_list_find(neo_list self,void *data);

void neo_list_remove(neo_list self, neo_list_node position);

void *neo_list_node_get(neo_list_node self);
void neo_list_node_set(neo_list_node self, void *data);

const size_t neo_list_length(neo_list self);
#endif