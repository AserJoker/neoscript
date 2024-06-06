#include "resolver/logic_or.h"
#include "engine/include/context.h"
#include "engine/include/type.h"
#include "engine/include/type/boolean.h"
#include "engine/include/type/exception.h"
#include "engine/include/typedef.h"
#include "engine/include/value.h"
#include "vm.h"
neo_value neo_resolver_logic_or(neo_vm vm, neo_ast node) {
  neo_context ctx = neo_vm_get_context(vm);
  if (!node->right || !node->left) {
    neo_context_throw(
        ctx, create_neo_exception(ctx, "Invalid operator logic_or", NULL,
                                  node->start.filename, node->start.line,
                                  node->start.column));
  }
  neo_value left = neo_vm_eval(vm, node->left);
  int8_t val = NEO_FALSE;
  if (neo_value_convert(left, NEO_TYPE_BOOLEAN, &val)) {
    return left;
  }
  neo_value right = neo_vm_eval(vm, node->right);
  if (neo_value_convert(right, NEO_TYPE_BOOLEAN, &val)) {
    return right;
  }
  return neo_context_get_null(ctx);
}