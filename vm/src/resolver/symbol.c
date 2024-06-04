#include "resolver/symbol.h"
#include "vm.h"
neo_value neo_resolver_symbol(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  neo_scope scope = neo_context_get_scope(ctx);
  neo_value value = neo_scope_query_value(scope, node->s_symbol);
  if (!value) {
    return neo_context_get_null(ctx);
  }
  return value;
}