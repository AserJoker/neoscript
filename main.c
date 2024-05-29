#include "ast/ast.h"
#include "context.h"
#include "resolver/integer.h"
#include "resolverdef.h"
#include "runtime.h"
#include "scope.h"
#include "type.h"
#include "type/array.h"
#include "type/boolean.h"
#include "type/custom.h"
#include "type/int16.h"
#include "type/int32.h"
#include "type/int64.h"
#include "type/int8.h"
#include "type/object.h"
#include "type/string.h"
#include "type/uint16.h"
#include "type/uint32.h"
#include "type/uint64.h"
#include "type/uint8.h"
#include "vm.h"
#include <stdint.h>
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
  neo_init_string(rt);

  neo_init_object(rt);
  neo_init_array(rt);

  neo_init_custom(rt);
  neo_context ctx = create_neo_context(rt);
  neo_vm vm = create_neo_vm(ctx);

  neo_vm_set_resolver(vm, NEO_RESOLVER_TYPE_INTEGER, neo_resolver_integer);
  neo_ast ast = create_neo_integer_ast(123);
  neo_value value = neo_vm_eval(vm, ast);
  free_neo_ast(ast);
  free_neo_vm(vm);
  free_neo_context(ctx);
  free_neo_runtime(rt);
  return 0;
}