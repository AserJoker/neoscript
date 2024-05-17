#include "engine/atom.h"
#include "util/list.h"

#include "engine/scope.h"
#include "engine/value.h"
#include <stdlib.h>
struct _neo_scope {
  neo_list values;
  neo_scope parent;
  neo_atom root;
};

neo_scope create_neo_scope(neo_scope parent) {
  neo_scope scope = (neo_scope)malloc(sizeof(struct _neo_scope));
  scope->parent = parent;
  scope->values = create_neo_list((neo_free_fn)free_neo_value);
  scope->root = create_neo_atom(NULL);
  return scope;
}
void free_neo_scope(neo_scope scope) {
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
  neo_list_remove(self->values, node);
  neo_atom_remove_ref(atom, self->root);
}

neo_atom neo_scope_get_root(neo_scope self) { return self->root; }