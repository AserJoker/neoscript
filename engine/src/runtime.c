#include "runtime.h"
#include "list.h"
#include "type.h"
#include "type/closure.h"
#include "type/exception.h"
#include "type/null.h"
#include "type/promise.h"
#include <assert.h>
#include <stdlib.h>

struct _neo_runtime {
  neo_list types;
};

neo_runtime create_neo_runtime() {
  neo_runtime rt = (neo_runtime)malloc(sizeof(struct _neo_runtime));
  assert(rt != NULL);
  rt->types = create_neo_list((neo_free_fn)free_neo_type);

  neo_null_init(rt);
  neo_init_exception(rt);
  neo_init_closure(rt);
  neo_init_promise(rt);
  return rt;
}

void free_neo_runtime(neo_runtime runtime) {
  if (!runtime) {
    return;
  }
  free_neo_list(runtime->types);
  free(runtime);
}

void neo_runtime_define_type(neo_runtime self, neo_type type) {
  neo_list_node node = neo_list_node_next(neo_list_head(self->types));
  while (node != neo_list_tail(self->types)) {
    neo_type t = (neo_type)neo_list_node_get(node);
    if (t == type) {
      return;
    }
    if (neo_type_get_name(t) == neo_type_get_name(type)) {
      neo_list_remove(self->types, node);
      node = neo_list_head(self->types);
    }
    node = neo_list_node_next(node);
  }
  neo_list_push(self->types, type);
}

neo_type neo_runtime_get_type(neo_runtime self, uint32_t name) {
  neo_list_node node = neo_list_node_next(neo_list_head(self->types));
  while (node != neo_list_tail(self->types)) {
    neo_type t = (neo_type)neo_list_node_get(node);
    if (neo_type_get_name(t) == name) {
      return t;
    }
    node = neo_list_node_next(node);
  }
  return NULL;
}