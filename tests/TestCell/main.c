
#include "engine/include/context.h"
#include "engine/include/type/closure.h"
#include <assert.h>
#define IS_CALLED 1
int is_called = 0;
neo_value neo_demo_func(neo_context ctx, size_t argc, neo_value *argv) {
  is_called = 1;
  return neo_context_get_null(ctx);
}
int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_context ctx = create_neo_context(rt);
  neo_value closure = create_neo_closure(ctx, neo_demo_func, "demo_func");
  neo_context_call(ctx, closure, 0, 0, __FILE__, __LINE__, 1);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  assert(is_called == IS_CALLED);
  return 0;
}