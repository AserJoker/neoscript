#include "resolver/comma.h"
#include "engine/include/type/exception.h"
#include "engine/include/value.h"
neo_value neo_resolver_comma(neo_vm vm, neo_ast node) {

  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(ctx, create_neo_exception(ctx, "Invalid operator comma",
                                                NULL, node->start.filename,
                                                node->start.line,
                                                node->start.column));
  }
  neo_vm_eval(vm, node->left);
  return neo_vm_eval(vm, node->right);
}