#include "engine/context.h"
#include "engine/atom.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/type/null.h"
#include "engine/value.h"
#include "util/list.h"
#include <stdlib.h>
struct _neo_context {
  neo_runtime rt;
  neo_scope scope;
  neo_value null;
};
neo_context create_neo_context(neo_runtime rt) {
  neo_context ctx = (neo_context)malloc(sizeof(struct _neo_context));
  ctx->rt = rt;
  ctx->scope = create_neo_scope(NULL);

  ctx->null = create_neo_null(ctx);
  return ctx;
}
void free_neo_context(neo_context ctx) {
  while (ctx->scope) {
    neo_context_pop_scope(ctx);
  }
  free(ctx);
}
neo_runtime neo_context_get_runtime(neo_context self) { return self->rt; }

void neo_context_push_scope(neo_context self) {
  neo_scope scope = create_neo_scope(self->scope);
  self->scope = scope;
}
void neo_context_pop_scope(neo_context self) {
  neo_scope parent = neo_scope_get_parent(self->scope);
  free_neo_scope(self->scope);
  self->scope = parent;
}

neo_scope neo_context_get_scope(neo_context self) { return self->scope; }
neo_value neo_context_create_value(neo_context self, neo_type type,
                                   void *init) {
  neo_atom atom = create_neo_atom(type, init);
  neo_value value = create_neo_value(self->scope, atom);
  return value;
}
neo_value neo_context_call(neo_context self, neo_function func, neo_value *args,
                           int argv) {
  neo_scope current = neo_context_get_scope(self);
  neo_context_push_scope(self);
  neo_value res = func(self, args, argv);
  if (neo_value_get_type_name(res) == NEO_TYPE_EXCEPTION) {
    // TODO: catch block trigger;
  }
  neo_value result = neo_scope_clone_value(current, res);
  neo_context_pop_scope(self);
  return result;
}
neo_value neo_context_get_null(neo_context ctx) { return ctx->null; }