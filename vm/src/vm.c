#include "vm.h"
#include "context.h"
#include "imap.h"
#include "type.h"
#include "type/exception.h"
#include <stdlib.h>
struct _neo_vm {
  neo_context ctx;
  neo_imap resolvers;
};
neo_vm create_neo_vm(neo_context ctx) {
  neo_vm vm = malloc(sizeof(struct _neo_vm));
  vm->resolvers = create_neo_imap(NULL);
  vm->ctx = ctx;
  return vm;
}
neo_context neo_vm_get_context(neo_vm self) { return self->ctx; }
void neo_vm_set_context(neo_vm self, neo_context ctx) { self->ctx = ctx; }
void free_neo_vm(neo_vm vm) {
  free_neo_imap(vm->resolvers);
  free(vm);
}
neo_value neo_vm_eval(neo_vm self, neo_ast ast) {
  neo_vm_resolver resolver = neo_imap_get(self->resolvers, ast->type);
  if (resolver) {
    return resolver(self, ast);
  }
  neo_context_throw_exception(self->ctx, "cannot resolve node");
  return NULL;
}
void neo_vm_set_resolver(neo_vm self, int32_t type, neo_vm_resolver resolver) {
  neo_imap_set(self->resolvers, type, resolver);
}