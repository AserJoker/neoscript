#include "engine/value.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "util/list.h"
#include <stdlib.h>
struct _neo_value {
  neo_atom atom;
  neo_scope scope;
};

neo_value create_neo_value(neo_scope current, neo_atom atom) {
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

neo_atom neo_value_get_atom(neo_value self) { return self->atom; }

void neo_value_set_atom(neo_value self, neo_atom atom) {
  if (self->atom != atom) {
    neo_atom root = neo_scope_get_root(self->scope);
    neo_atom_add_ref(atom, root);
    neo_atom_remove_ref(self->atom, root);
    self->atom = atom;
  }
}

neo_type neo_value_get_type(neo_value self) {
  return neo_atom_get_type(self->atom);
}

void *neo_value_get_data(neo_value self) { return neo_atom_get(self->atom); }