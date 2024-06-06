#include "resolver/number.h"
#include "engine/include/type/number.h"
#include "vm.h"
neo_value neo_resolver_number(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  return create_neo_number(ctx, node->f_data);
}