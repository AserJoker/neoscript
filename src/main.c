#include "engine/closure.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/value.h"
#include "util/list.h"
#include <stdint.h>
#include <stdio.h>

neo_closure neo_func1 = NULL;
neo_closure neo_func2 = NULL;

neo_value func2(neo_context ctx, size_t argc, neo_value *argv) {
  neo_list trace = neo_context_trace(ctx, __FILE__, __LINE__, 1);
  neo_list_node node = neo_list_head(trace);
  while (node != neo_list_tail(trace)) {
    const char *trace = (const char *)neo_list_node_get(node);
    if (trace) {
      printf("\t at %s\n", trace);
    }
    node = neo_list_node_next(node);
  }
  free_neo_list(trace);
  return neo_context_get_null(ctx);
}

neo_value func1(neo_context ctx, size_t argc, neo_value *argv) {
  neo_context_call(ctx, neo_func2, 0, 0, __FILE__, __LINE__, 1);
  return neo_context_get_null(ctx);
}

int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_context ctx = create_neo_context(rt);
  neo_func1 = create_neo_closure(ctx, func1, "func1");
  neo_func2 = create_neo_closure(ctx, func2, "func2");
  neo_context_call(ctx, neo_func1, 0, 0, __FILE__, __LINE__, 1);
  free_neo_closure(neo_func1);
  free_neo_closure(neo_func2);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}
