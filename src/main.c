#include "engine/atom.h"
#include "engine/closure.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/value.h"
#include "util/list.h"
#include <stdint.h>
#include <stdio.h>

#define NEO_TYPE_INT8 1
#define NEO_OPT_ADD 2
neo_value neo_print(neo_context ctx, size_t argc, neo_value *args) {
  neo_value arg = args[0];
  int8_t *val = (int8_t *)neo_value_get_data(arg);
  printf("%d\n", *val);
  return neo_context_get_null(ctx);
}
neo_value create_neo_int8(neo_context ctx, int8_t val) {
  neo_type neo_int8 =
      neo_runtime_get_type(neo_context_get_runtime(ctx), NEO_TYPE_INT8);
  return neo_context_create_value(ctx, neo_int8, &val);
}
neo_value neo_add(neo_context ctx, uint32_t opt, int argc, neo_value *argv) {
  neo_value val1 = argv[0];
  neo_value val2 = argv[1];
  int8_t *v1 = (int8_t *)neo_value_get_data(val1);
  int8_t *v2 = (int8_t *)neo_value_get_data(val2);
  int8_t v3 = *v1 + *v2;
  return create_neo_int8(ctx, v3);
}

int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_type neo_int8 = create_neo_type(NEO_TYPE_INT8, sizeof(int8_t), NULL);
  neo_runtime_define_type(rt, neo_int8);
  neo_runtime_define_operator(rt, NEO_OPT_ADD, neo_add);
  neo_context ctx = create_neo_context(rt);

  neo_value val1 = create_neo_int8(ctx, 1);
  neo_value val2 = create_neo_int8(ctx, 2);

  neo_value add_argv[] = {val1, val2};
  neo_value val3 = neo_context_operator(ctx, NEO_OPT_ADD, 2, add_argv);
  neo_closure print = create_neo_closure(ctx, neo_print);
  neo_context_call(ctx, print, &val3, 1);

  free_neo_closure(print);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}
