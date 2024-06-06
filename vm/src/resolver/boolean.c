#include "resolver/boolean.h"
#include "engine/include/type.h"
#include "engine/include/type/boolean.h"
#include "vm.h"
neo_value neo_resolver_boolean(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  return create_neo_boolean(ctx, node->b_data);
}