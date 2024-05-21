#ifndef __NEO_VM_VM__
#define __NEO_VM_VM__
#include "engine/runtime.h"
#include "engine/type.h"
typedef struct _neo_vm *neo_vm;

typedef uint16_t *(*neo_vm_opterator)(neo_vm vm, uint16_t *bytecodes);

neo_vm create_neo_vm(neo_runtime runtime);
void free_neo_vm(neo_vm vm);

neo_context neo_vm_get_context(neo_vm self);
neo_value neo_vm_eval(neo_vm vm, uint16_t *bytecodes);

void neo_vm_set_operators(neo_vm vm, neo_vm_opterator *operators);

void neo_vm_store_value(neo_vm vm, neo_value value, int32_t name);
neo_value neo_vm_query_value(neo_vm vm, int32_t name);

#endif