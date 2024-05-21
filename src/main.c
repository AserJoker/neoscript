#include "engine/closure.h"
#include "engine/context.h"
#include "engine/runtime.h"
#include "engine/type.h"
#include "vm/type/boolean.h"
#include "vm/type/int16.h"
#include "vm/type/int32.h"
#include "vm/type/int64.h"
#include "vm/type/int8.h"
#include "vm/type/uint16.h"
#include "vm/type/uint32.h"
#include "vm/type/uint64.h"
#include "vm/type/uint8.h"
#include "vm/vm.h"
#include <stdint.h>

neo_value neo_main(neo_context ctx, size_t argc, neo_value *argv) {
  neo_value a = create_neo_int32(ctx, 2);
  neo_value b = create_neo_int32(ctx, 3);
  return neo_context_get_null(ctx);
}

int main(int argc, char *argv[]) {
  neo_runtime rt = create_neo_runtime();

  neo_init_int8(rt);
  neo_init_int16(rt);
  neo_init_int32(rt);
  neo_init_int64(rt);

  neo_init_uint8(rt);
  neo_init_uint16(rt);
  neo_init_uint32(rt);
  neo_init_uint64(rt);

  neo_init_boolean(rt);
  neo_vm vm = create_neo_vm(rt);
  neo_context ctx = neo_vm_get_context(vm);
  neo_closure closure = create_neo_closure(ctx, neo_main, "neo_main");
  neo_context_call(ctx, closure, 0, 0, __FILE__, __LINE__, 1);
  free_neo_closure(closure);
  free_neo_vm(vm);
  free_neo_runtime(rt);
  return 0;
}
