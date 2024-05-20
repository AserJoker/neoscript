#include "engine/closure.h"
#include "engine/atom.h"
#include "engine/context.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/value.h"
#include "util/list.h"
#include "util/strings.h"
#include <assert.h>
#include <stdlib.h>

struct _neo_closure {
  neo_function func;
  neo_list values;
  neo_atom root;
  char *name;
};

neo_closure create_neo_closure(neo_context ctx, neo_function func,
                               const char *name) {
  neo_closure closure = (neo_closure)malloc(sizeof(struct _neo_closure));
  assert(closure != NULL);
  closure->func = func;
  closure->root = create_neo_atom(NULL, NULL);
  closure->values = create_neo_list((neo_free_fn)free_neo_atom);
  closure->name = strings_clone(name);
  return closure;
}
void free_neo_closure(neo_closure closure) {
  if (!closure) {
    return;
  }
  if (closure->name) {
    free(closure->name);
  }
  free_neo_list(closure->values);
  free_neo_atom(closure->root);
  free(closure);
}

neo_function neo_closure_get_function(neo_closure self) { return self->func; }
int32_t neo_closure_add(neo_closure self, neo_value val) {
  neo_atom atom = neo_value_get_atom(val);
  neo_atom_add_ref(atom, self->root);
  int32_t index = neo_list_length(self->values);
  neo_list_push(self->values, atom);
  return index;
}
neo_value neo_closure_get(neo_context ctx, neo_closure self, int32_t index) {
  neo_list_node node = neo_list_node_next(neo_list_head(self->values));
  for (int i = 0; i < index; i++) {
    if (node == neo_list_tail(self->values)) {
      return neo_context_get_null(ctx);
    }
    node = neo_list_node_next(node);
  }
  neo_atom atom = neo_list_node_get(node);
  if (atom) {
    return create_neo_value(neo_context_get_scope(ctx), atom);
  }
  return neo_context_get_null(ctx);
}
const char *neo_closure_get_name(neo_closure self) { return self->name; }