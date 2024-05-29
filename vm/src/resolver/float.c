#include "resolver/float.h"

#include "type.h"
#include "type/float64.h"
#include "vm.h"

neo_value neo_resolver_float(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  return create_neo_float64(ctx, node->f_data);
}