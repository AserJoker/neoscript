#include "resolver/set.h"
#include "engine/include/type.h"
#include "engine/include/type/exception.h"
#include "engine/include/value.h"
neo_value neo_resolver_set(neo_vm vm, neo_ast node) {

  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator comma",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_value left = neo_vm_eval(vm, node->left);
  neo_value right = neo_vm_eval(vm, node->right);
  neo_atom right_atom = neo_value_get_atom(right);
  neo_type type = neo_value_get_type(right);
  neo_type_hook *hook = neo_type_get_hook(type);
  if (hook->copy) {
    neo_value_set_atom(left, hook->copy(right_atom, hook->copy_arg));
  } else {
    neo_value_set_atom(left, right_atom);
  }
  return left;
}