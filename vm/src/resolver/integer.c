#include "resolver/integer.h"
#include "type.h"
#include "type/integer.h"
#include "vm.h"

neo_value neo_resolver_integer(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  return create_neo_integer(neo_vm_get_context(vm), node->i_data);
}