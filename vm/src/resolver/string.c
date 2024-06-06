
#include "resolver/string.h"
#include "engine/include/type.h"
#include "engine/include/type/string.h"
#include "vm.h"
neo_value neo_resolver_string(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  return create_neo_string(ctx, node->s_data);
}