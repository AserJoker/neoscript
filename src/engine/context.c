#include "engine/context.h"
#include "engine/value.h"
#include "util/list.h"
#include <stdlib.h>
struct _neo_context {
  neo_runtime rt;
  neo_scope scope;
};
neo_context create_neo_context(neo_runtime rt) {
  neo_context ctx = (neo_context)malloc(sizeof(struct _neo_context));
  ctx->rt = rt;
  ctx->scope = create_neo_scope(NULL);
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