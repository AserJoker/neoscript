#include "engine/closure.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/value.h"
#include "util/list.h"
#include "util/strings.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define NEO_TYPE_EXCEPTION 1

typedef struct _neo_exception_impl {
  char *message;
  neo_list trace;
} *neo_exception_impl;

void neo_string_init(void *target, void *source, void *_) {
  neo_exception_impl dst = (neo_exception_impl)target;
  neo_exception_impl src = (neo_exception_impl)source;
  dst->message = strings_clone(src->message);
  dst->trace = src->trace;
}
void neo_string_dispose(void *buf, void *_) {
  neo_exception_impl s = (neo_exception_impl)buf;
  free_neo_list(s->trace);
  free(s->message);
}

neo_closure neo_func1 = NULL;
neo_closure neo_func2 = NULL;

neo_value func2(neo_context ctx, size_t argc, neo_value *argv) {
  struct _neo_exception_impl s = {
      (char *)"demo error", neo_context_trace(ctx, __FILE__, __LINE__, 1)};
  neo_type neo_exception =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_EXCEPTION);
  neo_context_throw(ctx, neo_context_create_value(ctx, neo_exception, &s));

  return neo_context_get_null(ctx);
}

neo_value func1(neo_context ctx, size_t argc, neo_value *argv) {
  neo_context_call(ctx, neo_func2, 0, 0, __FILE__, __LINE__, 1);
  return neo_context_get_null(ctx);
}
void neo_error(neo_context ctx, neo_value error, void *_) {
  neo_value exception = error;
  neo_exception_impl impl = (neo_exception_impl)neo_value_get_data(exception);
  printf("Error: %s\n", impl->message);
  neo_list trace = impl->trace;
  neo_list_node node = neo_list_head(trace);
  while (node != neo_list_tail(trace)) {
    const char *frame = (const char *)neo_list_node_get(node);
    if (frame) {
      printf("\tat %s\n", frame);
    }
    node = neo_list_node_next(node);
  }
}

int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_type_hook hook = {neo_string_init, 0, neo_string_dispose, 0};
  neo_type neo_string = create_neo_type(
      NEO_TYPE_EXCEPTION, sizeof(struct _neo_exception_impl), &hook);
  neo_runtime_define_type(rt, neo_string);
  neo_context ctx = create_neo_context(rt);
  neo_context_set_error_callback(ctx, neo_error, NULL);

  neo_func1 = create_neo_closure(ctx, func1, "func1");
  neo_func2 = create_neo_closure(ctx, func2, "func2");

  neo_context_call(ctx, neo_func1, 0, 0, __FILE__, __LINE__, 1);

  free_neo_closure(neo_func1);
  free_neo_closure(neo_func2);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}
