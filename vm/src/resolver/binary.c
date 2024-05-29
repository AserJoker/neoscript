#include "resolver/binary.h"
#include "context.h"
#include "value.h"
#include "vm.h"
neo_value neo_resolver_binary(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  neo_value left = NULL;
  if (node->left) {
    left = neo_vm_eval(vm, node->left);
  }
  neo_value right = NULL;
  if (node->right) {
    right = neo_vm_eval(vm, node->right);
  }
  neo_value args[2] = {left, right};
  return neo_context_operator(ctx, node->data, 2, args);
}