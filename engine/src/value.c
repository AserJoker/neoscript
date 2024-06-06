#include "value.h"
#include "atom.h"
#include "scope.h"
#include "type.h"
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
  if (!value) {
    return;
  }
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

int8_t neo_value_convert(neo_value self, uint32_t type, void *output) {
  neo_type value_type = neo_value_get_type(self);
  neo_type_hook *hook = neo_type_get_hook(value_type);
  if (hook->convert) {
    return hook->convert(neo_value_get_data(self), type, output,
                         hook->convert_arg);
  }
  return 0;
}