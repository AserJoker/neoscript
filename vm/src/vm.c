#include "vm.h"
#include "common/include/imap.h"
#include "common/include/list.h"
#include "compiler/include/astdef.h"
#include "engine/include/type/exception.h"
#include "resolver/boolean.h"
#include "resolver/null.h"
#include "resolver/number.h"
#include "resolver/string.h"
#include "resolver/symbol.h"
#include <stdlib.h>
struct _neo_vm {
  neo_context ctx;
  neo_imap resolvers;
};
neo_vm create_neo_vm(neo_context ctx) {
  neo_vm vm = malloc(sizeof(struct _neo_vm));
  vm->resolvers = create_neo_imap((neo_free_fn)free_neo_list);
  vm->ctx = ctx;
  neo_vm_set_resolver(vm, NEO_AST_TYPE_NULL, neo_resolver_null);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_BOOLEAN, neo_resolver_boolean);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_NUMBER, neo_resolver_number);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_STRING, neo_resolver_string);
  neo_vm_set_resolver(vm, NEO_AST_TYPE_SYMBOL, neo_resolver_symbol);
  return vm;
}
neo_context neo_vm_get_context(neo_vm self) { return self->ctx; }
void neo_vm_set_context(neo_vm self, neo_context ctx) { self->ctx = ctx; }
void free_neo_vm(neo_vm vm) {
  free_neo_imap(vm->resolvers);
  free(vm);
}
neo_value neo_vm_eval(neo_vm self, neo_ast ast) {
  neo_list resolvers = neo_imap_get(self->resolvers, ast->type);
  if (resolvers) {
    neo_list_node node = neo_list_tail(resolvers);
    while (node != neo_list_head(resolvers)) {
      neo_vm_resolver resolver = neo_list_node_get(node);
      if (resolver) {
        neo_value result = resolver(self, ast);
        if (result) {
          return result;
        }
      }
      node = neo_list_node_last(node);
    }
  }
  neo_context_throw_exception(self->ctx, "cannot resolve node");
  return NULL;
}
void neo_vm_set_resolver(neo_vm self, int32_t type, neo_vm_resolver resolver) {
  if (!neo_imap_get(self->resolvers, type)) {
    neo_imap_set(self->resolvers, type, create_neo_list(NULL));
  }
  neo_list resolvers = neo_imap_get(self->resolvers, type);
  neo_list_push(resolvers, resolver);
}