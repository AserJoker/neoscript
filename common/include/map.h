#ifndef __NEO_COMMON_MAP__
#define __NEO_COMMON_MAP__
#include "./list.h"
#include <stdint.h>
typedef struct _neo_map *neo_map;
typedef int8_t (*neo_compare_fn)(void *, void *);
neo_map create_neo_map(neo_compare_fn cmp, neo_free_fn keyfree,
                       neo_free_fn valuefree);
void free_neo_map(neo_map map);

neo_list neo_map_keys(neo_map self);
void *neo_map_get(neo_map self, void *key);
void neo_map_set(neo_map self, void *key, void *value);
int8_t neo_map_has(neo_map self, void *key);
void neo_map_delete(neo_map self, void *key);
size_t neo_map_length(neo_map self);
#endif