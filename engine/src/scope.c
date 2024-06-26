#include "atom.h"
#include "common/include/cstring.h"
#include "common/include/list.h"
#include "common/include/map.h"
#include "type.h"

#include "scope.h"
#include "value.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
struct _neo_scope {
  neo_list values;
  neo_scope parent;
  neo_atom root;
  neo_map named;
};

int8_t neo_string_compare(const cstring a, const cstring b) {
  return strcmp(a, b) == 0;
}

neo_scope create_neo_scope(neo_scope parent) {
  neo_scope scope = (neo_scope)malloc(sizeof(struct _neo_scope));
  assert(scope != NULL);
  scope->parent = parent;
  scope->values = create_neo_list((neo_free_fn)free_neo_value);
  scope->root = create_neo_atom(NULL, NULL);
  scope->named = create_neo_map((neo_compare_fn)neo_string_compare, free, NULL);
  return scope;
}
void free_neo_scope(neo_scope scope) {
  if (!scope) {
    return;
  }
  free_neo_map(scope->named);
  free_neo_list(scope->values);
  free_neo_atom(scope->root);
  free(scope);
}
neo_scope neo_scope_get_parent(neo_scope self) { return self->parent; }

void neo_scope_add_value(neo_scope self, neo_value value) {
  neo_list_push(self->values, value);
  neo_atom atom = neo_value_get_atom(value);
  neo_atom_add_ref(atom, self->root);
}
void neo_scope_remove_value(neo_scope self, neo_value value) {
  neo_atom atom = neo_value_get_atom(value);
  neo_list_node node = neo_list_find(self->values, value);
  if (node) {
    neo_list_remove(self->values, node);
  } else {
    free_neo_value(value);
    neo_atom_remove_ref(atom, self->root);
  }
}

neo_atom neo_scope_get_root(neo_scope self) { return self->root; }
neo_value neo_scope_clone_value(neo_scope self, neo_value value) {
  if (!value) {
    return NULL;
  }
  neo_atom atom = neo_value_get_atom(value);
  neo_type type = neo_atom_get_type(atom);
  neo_type_hook *hook = neo_type_get_hook(type);
  if (hook->copy) {
    return create_neo_value(self, hook->copy(atom, hook->copy_arg));
  }
  return create_neo_value(self, atom);
}
void neo_scope_store_value(neo_scope self, const cstring name,
                           neo_value value) {
  neo_map_set(self->named, cstring_clone(name), value);
}
neo_value neo_scope_load_value(neo_scope self, const cstring name) {
  return neo_map_get(self->named, (void *)name);
}
neo_value neo_scope_query_value(neo_scope self, const cstring name) {
  neo_scope scope = self;
  neo_value val = neo_scope_load_value(scope, name);
  while (!val) {
    scope = neo_scope_get_parent(scope);
    if (!scope) {
      return NULL;
    }
    val = neo_scope_load_value(scope, name);
  }
  return val;
}