#ifndef __NEO_VM_VM__
#define __NEO_VM_VM__
#include "ast/ast.h"
#include "type.h"
typedef struct _neo_vm *neo_vm;
typedef neo_value (*neo_vm_resolver)(neo_vm vm, neo_ast ast);
neo_vm create_neo_vm(neo_context ctx);
void free_neo_vm(neo_vm vm);
neo_context neo_vm_get_context(neo_vm self);
void neo_vm_set_context(neo_vm self, neo_context ctx);
neo_value neo_vm_eval(neo_vm self, neo_ast ast);
void neo_vm_set_resolver(neo_vm self, int32_t type, neo_vm_resolver resolver);
#endif