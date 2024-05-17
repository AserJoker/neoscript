#include "engine/value.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/scope.h"
#include "util/list.h"
#include <stdlib.h>
struct _neo_value {
  neo_atom atom;
  neo_scope scope;
};

neo_value create_neo_value(neo_context ctx, neo_atom atom) {
  neo_scope current = neo_context_get_scope(ctx);
  neo_value val = (neo_value)malloc(sizeof(struct _neo_value));
  val->atom = atom;
  neo_scope_add_value(current, val);
  val->scope = current;
  return val;
}
void free_neo_value(neo_value value) {
  if (value->scope) {
    neo_scope scope = value->scope;
    value->scope = NULL;
    neo_scope_remove_value(scope, value);
  } else {
    free_neo_atom(value->atom);
    free(value);
  }
}

neo_atom neo_value_get_atom(neo_value value) { return value->atom; }

neo_value neo_value_assign(neo_value self, neo_value another) {
  if (self->atom != another->atom) {
    neo_atom root = neo_scope_get_root(self->scope);
    neo_atom_add_ref(another->atom, root);
    neo_atom_remove_ref(self->atom, root);
    self->atom = another->atom;
  }
  return self;
}