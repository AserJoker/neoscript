#include "engine/type.h"
#include <stdlib.h>
#include <string.h>
struct _neo_type {
  int32_t name;
  size_t size;
  neo_type_hook hook;
};

neo_type create_neo_type(const int32_t name, const size_t size,
                         neo_type_hook *hook) {
  neo_type t = (neo_type)malloc(sizeof(struct _neo_type));
  t->name = name;
  t->size = size;
  if (hook) {
    t->hook = *hook;
  } else {
    memset(&t->hook, 0, sizeof(struct _neo_type_hook));
  }
  return t;
}

neo_type_hook neo_type_get_hook(neo_type type) { return type->hook; }
void free_neo_type(neo_type type) { free(type); }

const int32_t neo_type_get_name(neo_type self) { return self->name; }
const size_t neo_type_get_size(neo_type self) { return self->size; }