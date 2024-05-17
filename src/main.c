#include "engine/atom.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/scope.h"
#include "engine/type.h"
#include "engine/value.h"
#include "util/list.h"
#include <stdint.h>
#define NEO_TYPE_INT8 1
int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();
  neo_type neo_int8 = create_neo_type(NEO_TYPE_INT8, sizeof(int8_t), NULL);
  neo_runtime_define_type(rt, neo_int8);
  neo_context ctx = create_neo_context(rt);

  neo_value val = neo_context_create_value(ctx, neo_int8, NULL);
  neo_value val2 = neo_context_create_value(ctx, neo_int8, NULL);

  neo_atom atom1 = neo_value_get_atom(val);
  neo_atom atom2 = neo_value_get_atom(val2);

  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}
