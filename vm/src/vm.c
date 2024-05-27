#include "vm.h"
#include "context.h"
#include "imap.h"
#include "value.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct _neo_vm {
  neo_imap values;
  neo_vm_opterator *operators;
  neo_runtime runtime;
  neo_context context;
};

neo_vm create_neo_vm(neo_runtime runtime) {
  neo_vm vm = (neo_vm)malloc(sizeof(struct _neo_vm));
  vm->runtime = runtime;
  vm->context = create_neo_context(runtime);
  vm->operators = NULL;
  vm->values = create_neo_imap(NULL);

  return vm;
}

void free_neo_vm(neo_vm vm) {
  free_neo_imap(vm->values);
  free_neo_context(vm->context);
  free(vm);
}

neo_context neo_vm_get_context(neo_vm self) { return self->context; }

neo_value neo_vm_eval(neo_vm self, uint16_t *bytecodes) {
  uint16_t *selector = bytecodes;
  while (selector) {
    neo_vm_opterator oper = self->operators[*selector];
    if (oper) {
      selector = oper(self, selector + 1);
    } else {
      fprintf(stderr, "cannot resolve operator code: 0x%x", *selector);
      return NULL;
    }
  }
  return neo_context_get_null(self->context);
}

void neo_vm_set_operators(neo_vm self, neo_vm_opterator *operators) {
  self->operators = operators;
}

void neo_vm_store_value(neo_vm self, neo_value value, int32_t name) {
  neo_imap_set(self->values, name, value);
}
neo_value neo_vm_query_value(neo_vm self, int32_t name) {
  return (neo_value)neo_imap_get(self->values, name);
}