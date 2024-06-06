#include "resolver/null.h"
#include "engine/include/context.h"
#include "engine/include/type.h"
#include "vm.h"
neo_value neo_resolver_null(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  return neo_context_get_null(ctx);
}