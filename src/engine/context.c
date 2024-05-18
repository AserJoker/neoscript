#include "engine/context.h"
#include "engine/atom.h"
#include "engine/closure.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/type/exception.h"
#include "engine/type/null.h"
#include "engine/value.h"
#include "util/list.h"
#include <stdlib.h>
struct _neo_context {
  neo_runtime rt;
  neo_scope scope;
  neo_list closures;
  neo_value null;
};
neo_context create_neo_context(neo_runtime rt) {
  neo_context ctx = (neo_context)malloc(sizeof(struct _neo_context));
  ctx->rt = rt;
  ctx->scope = create_neo_scope(NULL);
  ctx->closures = create_neo_list(NULL);

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
neo_value neo_context_call(neo_context self, neo_closure closure,
                           neo_value *args, int argc) {
  neo_scope current = neo_context_get_scope(self);
  neo_function func = neo_closure_get_function(closure);
  neo_list_push(self->closures, closure);
  neo_context_push_scope(self);
  neo_value *args_next = (neo_value *)malloc(sizeof(neo_value *) * argc);
  neo_scope func_current = neo_context_get_scope(self);
  for (int i = 0; i < argc; i++) {
    args_next[i] = neo_scope_clone_value(func_current, args[i]);
  }
  neo_value res = func(self, argc, args_next);
  free(args_next);
  if (neo_value_get_type_name(res) == NEO_TYPE_EXCEPTION) {
    // TODO: catch block trigger;
  }
  neo_value result = neo_scope_clone_value(current, res);
  neo_list_pop(self->closures);
  neo_context_pop_scope(self);
  return result;
}
neo_value neo_context_get_null(neo_context ctx) { return ctx->null; }

neo_value neo_context_get_closure_value(neo_context ctx, int cindex,
                                        int index) {
  neo_list_node node = neo_list_node_next(neo_list_head(ctx->closures));
  for (int i = 0; i < cindex; i++) {
    if (node == neo_list_tail(ctx->closures)) {
      return neo_context_get_null(ctx);
    }
    node = neo_list_node_next(node);
  }
  if (node == neo_list_tail(ctx->closures)) {
    return neo_context_get_null(ctx);
  }
  neo_closure closure = (neo_closure)neo_list_node_get(node);
  return neo_closure_get(ctx, closure, index);
}
neo_value neo_context_operator(neo_context self, uint32_t opt, int argc,
                               neo_value *argv) {
  neo_operator_fn operator= neo_runtime_get_operator(self->rt, opt);
  if (operator) {
    return operator(self, opt, argc, argv);
  }
  return NULL;
}